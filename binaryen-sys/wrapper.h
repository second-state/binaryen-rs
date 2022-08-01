#include <stdbool.h>

#include "binaryen/src/binaryen-c.h"

#ifdef __cplusplus
extern "C" {
#endif

BINARYEN_REF(PassOptions);

BinaryenPassOptionsRef BinaryenPassOptionsCreate(void);

void BinaryenPassOptionsDispose(BinaryenPassOptionsRef passOptions);

void BinaryenPassOptionsSetArgument(BinaryenPassOptionsRef passOptions, const char *key, const char *value);

void BinaryenPassOptionsSetOptimizationOptions(BinaryenPassOptionsRef passOptions, int shrinkLevel, int optimizeLevel, int debugInfo);

BinaryenModuleRef BinaryenModuleSafeRead(const char* input, size_t inputSize);

void BinaryenShimDisposeBinaryenModuleAllocateAndWriteResult(
    BinaryenModuleAllocateAndWriteResult result
);

void BinaryenModuleRunPassesWithSettings(
    BinaryenModuleRef module, const char** passes, BinaryenIndex numPasses,
    BinaryenPassOptionsRef passOptions
);

int BinaryenModuleSafeValidate(BinaryenModuleRef module);

#ifdef __cplusplus
}
#endif
