#include <gtest/gtest.h>
#include <cstring>
#include <vector>

// Include the production header that declares OPENFILENAME and the dialog functions
#include "GDI/Comdlg32.h"

// We need to test that memcpy never copies more than sizeof(OPENFILENAME) bytes,
// regardless of what lStructSize claims.

class Comdlg32BufferOverflowTest : public ::testing::TestWithParam<DWORD> {};

TEST_P(Comdlg32BufferOverflowTest, LStructSizeNeverExceedsStructBounds) {
    // Invariant: Buffer reads never exceed sizeof(OPENFILENAME), even if
    // lStructSize is set to a larger value by the caller.
    DWORD malicious_size = GetParam();

    // Allocate a large buffer to simulate a crafted OPENFILENAME with inflated lStructSize
    std::vector<uint8_t> buf(malicious_size, 0x41);
    OPENFILENAME* crafted = reinterpret_cast<OPENFILENAME*>(buf.data());
    crafted->lStructSize = malicious_size;

    // Call GetOpenFileName which internally does memcpy(&OpenFile, lpOpenFile, lpOpenFile->lStructSize)
    // If the fix is in place, it should clamp or reject oversized lStructSize.
    // We expect the function to either return FALSE (reject) or not crash (clamp).
    BOOL result = GetOpenFileName(crafted);

    // The function should not succeed with a bogus structure, and must not have crashed
    // (if we reach here, no stack smash occurred)
    EXPECT_FALSE(result);
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    Comdlg32BufferOverflowTest,
    ::testing::Values(
        sizeof(OPENFILENAME),          // Valid: exact size (boundary, should work normally)
        sizeof(OPENFILENAME) * 2,      // Exploit: 2x overflow
        sizeof(OPENFILENAME) * 10,     // Extreme: 10x overflow
        0xFFFF,                        // Large arbitrary value
        0                              // Zero size edge case
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}