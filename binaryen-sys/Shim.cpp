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
    return BinaryenModuleRead((char*)input, inputSize);
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

// NOTE: this is based on BinaryenModuleRunPasses and BinaryenModuleOptimizer
// from binaryen-c.cpp
// Main benefit is being thread safe.
extern "C" void BinaryenModuleRunPassesWithSettings(
    BinaryenModuleRef module, const char** passes, BinaryenIndex numPasses,
    int shrinkLevel, int optimizeLevel, int debugInfo
) {
  Module* wasm = (Module*)module;
  PassRunner passRunner(wasm);
  passRunner.options = PassOptions::getWithDefaultOptimizationOptions();
  passRunner.options.shrinkLevel = shrinkLevel;
  passRunner.options.optimizeLevel = optimizeLevel;
  passRunner.options.debugInfo = debugInfo != 0;
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
