// Copyright (c) 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Validation tests for Data Rules.

#include <sstream>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "unit_spirv.h"
#include "val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::MatchesRegex;

using std::pair;
using std::string;
using std::stringstream;

using ValidateData = spvtest::ValidateBase<pair<string, bool>>;

string HeaderWith(std::string cap) {
  return std::string("OpCapability Shader OpCapability Linkage OpCapability ") +
         cap + " OpMemoryModel Logical GLSL450 ";
}

string header = R"(
     OpCapability Shader
     OpCapability Linkage
     OpMemoryModel Logical GLSL450
)";
string header_with_addresses = R"(
     OpCapability Addresses
     OpCapability Kernel
     OpCapability GenericPointer
     OpCapability Linkage
     OpMemoryModel Physical32 OpenCL
)";
string header_with_vec16_cap = R"(
     OpCapability Shader
     OpCapability Vector16
     OpCapability Linkage
     OpMemoryModel Logical GLSL450
)";
string header_with_int8 = R"(
     OpCapability Shader
     OpCapability Linkage
     OpCapability Int8
     OpMemoryModel Logical GLSL450
)";
string header_with_int16 = R"(
     OpCapability Shader
     OpCapability Linkage
     OpCapability Int16
     OpMemoryModel Logical GLSL450
)";
string header_with_int64 = R"(
     OpCapability Shader
     OpCapability Linkage
     OpCapability Int64
     OpMemoryModel Logical GLSL450
)";
string header_with_float16 = R"(
     OpCapability Shader
     OpCapability Linkage
     OpCapability Float16
     OpMemoryModel Logical GLSL450
)";
string header_with_float16_buffer = R"(
     OpCapability Shader
     OpCapability Linkage
     OpCapability Float16Buffer
     OpMemoryModel Logical GLSL450
)";
string header_with_float64 = R"(
     OpCapability Shader
     OpCapability Linkage
     OpCapability Float64
     OpMemoryModel Logical GLSL450
)";

string invalid_comp_error = "Illegal number of components";
string missing_cap_error = "requires the Vector16 capability";
string missing_int8_cap_error = "requires the Int8 capability";
string missing_int16_cap_error =
    "requires the Int16 capability,"
    " or an extension that explicitly enables 16-bit integers.";
string missing_int64_cap_error = "requires the Int64 capability";
string missing_float16_cap_error =
    "requires the Float16 or Float16Buffer capability,"
    " or an extension that explicitly enables 16-bit floating point.";
string missing_float64_cap_error = "requires the Float64 capability";
string invalid_num_bits_error = "Invalid number of bits";

TEST_F(ValidateData, vec0) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 0
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(invalid_comp_error));
}

TEST_F(ValidateData, vec1) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 1
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(invalid_comp_error));
}

TEST_F(ValidateData, vec2) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 2
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, vec3) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 3
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, vec4) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, vec5) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 5
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(invalid_comp_error));
}

TEST_F(ValidateData, vec8) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 8
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(missing_cap_error));
}

TEST_F(ValidateData, vec8_with_capability) {
  string str = header_with_vec16_cap + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 8
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, vec16) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 8
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(missing_cap_error));
}

TEST_F(ValidateData, vec16_with_capability) {
  string str = header_with_vec16_cap + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 16
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, vec15) {
  string str = header + R"(
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 15
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(invalid_comp_error));
}

TEST_F(ValidateData, int8_good) {
  string str = header_with_int8 + "%2 = OpTypeInt 8 0";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, int8_bad) {
  string str = header + "%2 = OpTypeInt 8 1";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(missing_int8_cap_error));
}

TEST_F(ValidateData, int8_with_storage_buffer_8bit_access_good) {
  string str = HeaderWith(
                   "StorageBuffer8BitAccess "
                   "OpExtension \"SPV_KHR_8bit_storage\"") +
               " %2 = OpTypeInt 8 0";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions()) << getDiagnosticString();
}

TEST_F(ValidateData, int8_with_uniform_and_storage_buffer_8bit_access_good) {
  string str = HeaderWith(
                   "UniformAndStorageBuffer8BitAccess "
                   "OpExtension \"SPV_KHR_8bit_storage\"") +
               " %2 = OpTypeInt 8 0";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions()) << getDiagnosticString();
}

TEST_F(ValidateData, int8_with_storage_push_constant_8_good) {
  string str = HeaderWith(
                   "StoragePushConstant8 "
                   "OpExtension \"SPV_KHR_8bit_storage\"") +
               " %2 = OpTypeInt 8 0";
  CompileSuccessfully(str.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions()) << getDiagnosticString();
}

TEST_F(ValidateData, int16_good) {
  string str = header_with_int16 + "%2 = OpTypeInt 16 1";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, storage_uniform_buffer_block_16_good) {
  string str = HeaderWith(
                   "StorageUniformBufferBlock16 "
                   "OpExtension \"SPV_KHR_16bit_storage\"") +
               "%2 = OpTypeInt 16 1 %3 = OpTypeFloat 16";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, storage_uniform_16_good) {
  string str =
      HeaderWith("StorageUniform16 OpExtension \"SPV_KHR_16bit_storage\"") +
      "%2 = OpTypeInt 16 1 %3 = OpTypeFloat 16";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, storage_push_constant_16_good) {
  string str = HeaderWith(
                   "StoragePushConstant16 "
                   "OpExtension \"SPV_KHR_16bit_storage\"") +
               "%2 = OpTypeInt 16 1 %3 = OpTypeFloat 16";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, storage_input_output_16_good) {
  string str = HeaderWith(
                   "StorageInputOutput16 "
                   "OpExtension \"SPV_KHR_16bit_storage\"") +
               "%2 = OpTypeInt 16 1 %3 = OpTypeFloat 16";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, int16_bad) {
  string str = header + "%2 = OpTypeInt 16 1";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(missing_int16_cap_error));
}

TEST_F(ValidateData, int64_good) {
  string str = header_with_int64 + "%2 = OpTypeInt 64 1";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, int64_bad) {
  string str = header + "%2 = OpTypeInt 64 1";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(missing_int64_cap_error));
}

// Number of bits in an integer may be only one of: {8,16,32,64}
TEST_F(ValidateData, int_invalid_num_bits) {
  string str = header + "%2 = OpTypeInt 48 1";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(invalid_num_bits_error));
}

TEST_F(ValidateData, float16_good) {
  string str = header_with_float16 + "%2 = OpTypeFloat 16";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, float16_buffer_good) {
  string str = header_with_float16_buffer + "%2 = OpTypeFloat 16";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, float16_bad) {
  string str = header + "%2 = OpTypeFloat 16";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(missing_float16_cap_error));
}

TEST_F(ValidateData, float64_good) {
  string str = header_with_float64 + "%2 = OpTypeFloat 64";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, float64_bad) {
  string str = header + "%2 = OpTypeFloat 64";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(missing_float64_cap_error));
}

// Number of bits in a float may be only one of: {16,32,64}
TEST_F(ValidateData, float_invalid_num_bits) {
  string str = header + "%2 = OpTypeFloat 48";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(invalid_num_bits_error));
}

TEST_F(ValidateData, matrix_data_type_float) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%vec3   =  OpTypeVector %f32 3
%mat33  =  OpTypeMatrix %vec3 3
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, ids_should_be_validated_before_data) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%mat33  =  OpTypeMatrix %vec3 3
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("ID 3 has not been defined"));
}

TEST_F(ValidateData, matrix_bad_column_type) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%mat33  =  OpTypeMatrix %f32 3
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Columns in a matrix must be of type vector"));
}

TEST_F(ValidateData, matrix_data_type_int) {
  string str = header + R"(
%int32  =  OpTypeInt 32 1
%vec3   =  OpTypeVector %int32 3
%mat33  =  OpTypeMatrix %vec3 3
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("can only be parameterized with floating-point types"));
}

TEST_F(ValidateData, matrix_data_type_bool) {
  string str = header + R"(
%boolt  =  OpTypeBool
%vec3   =  OpTypeVector %boolt 3
%mat33  =  OpTypeMatrix %vec3 3
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("can only be parameterized with floating-point types"));
}

TEST_F(ValidateData, matrix_with_0_columns) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%vec3   =  OpTypeVector %f32 3
%mat33  =  OpTypeMatrix %vec3 0
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("can only be parameterized as having only 2, 3, or 4 columns"));
}

TEST_F(ValidateData, matrix_with_1_column) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%vec3   =  OpTypeVector %f32 3
%mat33  =  OpTypeMatrix %vec3 1
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("can only be parameterized as having only 2, 3, or 4 columns"));
}

TEST_F(ValidateData, matrix_with_2_columns) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%vec3   =  OpTypeVector %f32 3
%mat33  =  OpTypeMatrix %vec3 2
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, matrix_with_3_columns) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%vec3   =  OpTypeVector %f32 3
%mat33  =  OpTypeMatrix %vec3 3
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, matrix_with_4_columns) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%vec3   =  OpTypeVector %f32 3
%mat33  =  OpTypeMatrix %vec3 4
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, matrix_with_5_column) {
  string str = header + R"(
%f32    =  OpTypeFloat 32
%vec3   =  OpTypeVector %f32 3
%mat33  =  OpTypeMatrix %vec3 5
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("can only be parameterized as having only 2, 3, or 4 columns"));
}

TEST_F(ValidateData, specialize_int) {
  string str = header + R"(
%i32 = OpTypeInt 32 1
%len = OpSpecConstant %i32 2)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, specialize_float) {
  string str = header + R"(
%f32 = OpTypeFloat 32
%len = OpSpecConstant %f32 2)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, specialize_boolean) {
  string str = header + R"(
%2 = OpTypeBool
%3 = OpSpecConstantTrue %2
%4 = OpSpecConstantFalse %2)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, specialize_boolean_to_int) {
  string str = header + R"(
%2 = OpTypeInt 32 1
%3 = OpSpecConstantTrue %2
%4 = OpSpecConstantFalse %2)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Specialization constant must be a boolean"));
}

TEST_F(ValidateData, missing_forward_pointer_decl) {
  string str = header_with_addresses + R"(
%uintt = OpTypeInt 32 0
%3 = OpTypeStruct %fwd_ptrt %uintt
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must first be declared using OpTypeForwardPointer"));
}

TEST_F(ValidateData, forward_pointer_missing_definition) {
  string str = header_with_addresses + R"(
OpTypeForwardPointer %_ptr_Generic_struct_A Generic
%uintt = OpTypeInt 32 0
%struct_B = OpTypeStruct %uintt %_ptr_Generic_struct_A
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("forward referenced IDs have not been defined"));
}

TEST_F(ValidateData, forward_ref_bad_type) {
  string str = header_with_addresses + R"(
OpTypeForwardPointer %_ptr_Generic_struct_A Generic
%uintt = OpTypeInt 32 0
%struct_B = OpTypeStruct %uintt %_ptr_Generic_struct_A
%_ptr_Generic_struct_A = OpTypeFloat 32
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Found a forward reference to a non-pointer type in "
                        "OpTypeStruct instruction."));
}

TEST_F(ValidateData, forward_ref_points_to_non_struct) {
  string str = header_with_addresses + R"(
OpTypeForwardPointer %_ptr_Generic_struct_A Generic
%uintt = OpTypeInt 32 0
%struct_B = OpTypeStruct %uintt %_ptr_Generic_struct_A
%_ptr_Generic_struct_A = OpTypePointer Generic %uintt
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("A forward reference operand in an OpTypeStruct must "
                        "be an OpTypePointer that points to an OpTypeStruct. "
                        "Found OpTypePointer that points to OpTypeInt."));
}

TEST_F(ValidateData, struct_forward_pointer_good) {
  string str = header_with_addresses + R"(
OpTypeForwardPointer %_ptr_Generic_struct_A Generic
%uintt = OpTypeInt 32 0
%struct_B = OpTypeStruct %uintt %_ptr_Generic_struct_A
%struct_C = OpTypeStruct %uintt %struct_B
%struct_A = OpTypeStruct %uintt %struct_C
%_ptr_Generic_struct_A = OpTypePointer Generic %struct_C
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateData, ext_16bit_storage_caps_allow_free_fp_rounding_mode) {
  for (const char* cap : {"StorageUniform16", "StorageUniformBufferBlock16",
                          "StoragePushConstant16", "StorageInputOutput16"}) {
    for (const char* mode : {"RTE", "RTZ", "RTP", "RTN"}) {
      string str = string(R"(
        OpCapability Shader
        OpCapability Linkage
        OpCapability )") +
                   cap + R"(
        OpExtension "SPV_KHR_16bit_storage"
        OpMemoryModel Logical GLSL450
        OpDecorate %2 FPRoundingMode )" +
                   mode + R"(
        %1 = OpTypeFloat 32
        %2 = OpConstant %1 1.25
      )";
      CompileSuccessfully(str.c_str());
      ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
    }
  }
}

TEST_F(ValidateData, vulkan_disallow_free_fp_rounding_mode) {
  for (const char* mode : {"RTE", "RTZ"}) {
    for (const auto env : {SPV_ENV_VULKAN_1_0, SPV_ENV_VULKAN_1_1}) {
      string str = string(R"(
        OpCapability Shader
        OpMemoryModel Logical GLSL450
        OpDecorate %2 FPRoundingMode )") +
                   mode + R"(
        %1 = OpTypeFloat 32
        %2 = OpConstant %1 1.25
      )";
      CompileSuccessfully(str.c_str());
      ASSERT_EQ(SPV_ERROR_INVALID_CAPABILITY, ValidateInstructions(env));
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Operand 2 of Decorate requires one of these capabilities: "
                    "StorageBuffer16BitAccess StorageUniform16 "
                    "StoragePushConstant16 StorageInputOutput16"));
    }
  }
}

}  // namespace
}  // namespace val
}  // namespace spvtools
