#include <cstddef>
#include <cstring>

#include "wrapper.h"
#include "asm_v_wasm.h"
#include "support/file.h"
#include "pass.h"
#include "tools/optimization-options.h"
#include "tools/fuzzing.h"
#include "binaryen-c.h"

#include "wasm.h"           // For Feature enum
#include "wasm-validator.h" // For WasmValidator

#include "wasm-binary.h"    // For SafeRead

using namespace wasm;
using namespace std;

// NOTE: this is based on BinaryenModuleRead from binaryen-c.cpp
extern "C" BinaryenModuleRef BinaryenModuleSafeRead(const char* input, size_t inputSize) {
  auto* wasm = new Module;
  std::vector<char> buffer(false);
  buffer.resize(inputSize);
  std::copy_n(input, inputSize, buffer.begin());
  try {
    // TODO: allow providing features in the C API
    WasmBinaryBuilder parser(*wasm, FeatureSet::MVP, buffer);
    parser.read();
  } catch (ParseException& p) {
    // FIXME: support passing back the exception text
    return NULL;
  }
  return wasm;
}

extern "C" void BinaryenShimDisposeBinaryenModuleAllocateAndWriteResult(
    BinaryenModuleAllocateAndWriteResult result
) {
    if (result.binary) {
        free(result.binary);
    }
    if (result.sourceMap) {
        free(result.sourceMap);
    }
}

extern "C" BinaryenPassOptionsRef BinaryenPassOptionsCreate(void) {
  auto passOptions = new PassOptions();
  passOptions->setDefaultOptimizationOptions();
  return passOptions;
}

extern "C" void BinaryenPassOptionsSetOptimizationOptions(BinaryenPassOptionsRef passOptions, int shrinkLevel, int optimizeLevel, int debugInfo){
  passOptions->shrinkLevel = shrinkLevel;
  passOptions->optimizeLevel = optimizeLevel;
  passOptions->debugInfo = debugInfo;
}

extern "C" void BinaryenPassOptionsDispose(BinaryenPassOptionsRef passOptions) { delete (PassOptions*)passOptions; }

extern "C" void BinaryenPassOptionsSetArgument(BinaryenPassOptionsRef passOptions, const char *key, const char *value){
  assert(key);
  if (value){
    passOptions->arguments[key] = value;
  }
  else{
    passOptions->arguments.erase(key);
  }
}

// NOTE: this is based on BinaryenModuleRunPasses and BinaryenModuleOptimizer
// from binaryen-c.cpp
// Main benefit is being thread safe.
extern "C" void BinaryenModuleRunPassesWithSettings(
    BinaryenModuleRef module, const char **passes, BinaryenIndex numPasses,
    BinaryenPassOptionsRef passOptions
){
  Module* wasm = (Module*)module;
  PassRunner passRunner(wasm);
  passRunner.options = *passOptions;
  if (passes == nullptr) {
    passRunner.addDefaultOptimizationPasses();
  } else {
    for (BinaryenIndex i = 0; i < numPasses; i++) {
      passRunner.add(passes[i]);
    }
  }
  passRunner.run();
}

// NOTE: this is based on BinaryenModuleValidate from binaryen-c.cpp
extern "C" int BinaryenModuleSafeValidate(BinaryenModuleRef module) {
  Module* wasm = (Module*)module;
  auto features = wasm->features;
  // TODO(tlively): Add C API for managing features
  wasm->features = FeatureSet::All;
  auto ret = WasmValidator().validate(*wasm) ? 1 : 0;
  wasm->features = features;
  return ret;
}

extern "C" BinaryenFunctionRef BinaryenGetStart(BinaryenModuleRef module) {
  auto start = ((Module *)module)->start;
  return ((Module *)module)->getFunctionOrNull(start);
}