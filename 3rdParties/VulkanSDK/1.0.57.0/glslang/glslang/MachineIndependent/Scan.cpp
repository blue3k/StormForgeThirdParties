//
// Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
// Copyright (C) 2013 LunarG, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

//
// GLSL scanning, leveraging the scanning done by the preprocessor.
//

#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "../Include/Types.h"
#include "SymbolTable.h"
#include "ParseHelper.h"
#include "glslang_tab.cpp.h"
#include "ScanContext.h"
#include "Scan.h"

// preprocessor includes
#include "preprocessor/PpContext.h"
#include "preprocessor/PpTokens.h"

// Required to avoid missing prototype warnings for some compilers
int yylex(YYSTYPE*, glslang::TParseContext&);

namespace glslang {

// read past any white space
void TInputScanner::consumeWhiteSpace(bool& foundNonSpaceTab)
{
    int c = peek();  // don't accidentally consume anything other than whitespace
    while (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
        if (c == '\r' || c == '\n')
            foundNonSpaceTab = true;
        get();
        c = peek();
    }
}

// return true if a comment was actually consumed
bool TInputScanner::consumeComment()
{
    if (peek() != '/')
        return false;

    get();  // consume the '/'
    int c = peek();
    if (c == '/') {

        // a '//' style comment
        get();  // consume the second '/'
        c = get();
        do {
            while (c != EndOfInput && c != '\\' && c != '\r' && c != '\n')
                c = get();

            if (c == EndOfInput || c == '\r' || c == '\n') {
                while (c == '\r' || c == '\n')
                    c = get();

                // we reached the end of the comment
                break;
            } else {
                // it's a '\', so we need to keep going, after skipping what's escaped

                // read the skipped character
                c = get();

                // if it's a two-character newline, skip both characters
                if (c == '\r' && peek() == '\n')
                    get();
                c = get();
            }
        } while (true);

        // put back the last non-comment character
        if (c != EndOfInput)
            unget();

        return true;
    } else if (c == '*') {

        // a '/*' style comment
        get();  // consume the '*'
        c = get();
        do {
            while (c != EndOfInput && c != '*')
                c = get();
            if (c == '*') {
                c = get();
                if (c == '/')
                    break;  // end of comment
                // not end of comment
            } else // end of input
                break;
        } while (true);

        return true;
    } else {
        // it's not a comment, put the '/' back
        unget();

        return false;
    }
}

// skip whitespace, then skip a comment, rinse, repeat
void TInputScanner::consumeWhitespaceComment(bool& foundNonSpaceTab)
{
    do {
        consumeWhiteSpace(foundNonSpaceTab);

        // if not starting a comment now, then done
        int c = peek();
        if (c != '/' || c == EndOfInput)
            return;

        // skip potential comment
        foundNonSpaceTab = true;
        if (! consumeComment())
            return;

    } while (true);
}

// Returns true if there was non-white space (e.g., a comment, newline) before the #version
// or no #version was found; otherwise, returns false.  There is no error case, it always
// succeeds, but will leave version == 0 if no #version was found.
//
// Sets notFirstToken based on whether tokens (beyond white space and comments)
// appeared before the #version.
//
// N.B. does not attempt to leave input in any particular known state.  The assumption
// is that scanning will start anew, following the rules for the chosen version/profile,
// and with a corresponding parsing context.
//
bool TInputScanner::scanVersion(int& version, EProfile& profile, bool& notFirstToken)
{
    // This function doesn't have to get all the semantics correct,
    // just find the #version if there is a correct one present.
    // The preprocessor will have the responsibility of getting all the semantics right.

    bool versionNotFirst = false;  // means not first WRT comments and white space, nothing more
    notFirstToken = false;         // means not first WRT to real tokens
    version = 0;                   // means not found
    profile = ENoProfile;

    bool foundNonSpaceTab = false;
    bool lookingInMiddle = false;
    int c;
    do {
        if (lookingInMiddle) {
            notFirstToken = true;
            // make forward progress by finishing off the current line plus extra new lines
            if (peek() == '\n' || peek() == '\r') {
                while (peek() == '\n' || peek() == '\r')
                    get();
            } else
                do {
                    c = get();
                } while (c != EndOfInput && c != '\n' && c != '\r');
                while (peek() == '\n' || peek() == '\r')
                    get();
                if (peek() == EndOfInput)
                    return true;
        }
        lookingInMiddle = true;

        // Nominal start, skipping the desktop allowed comments and white space, but tracking if
        // something else was found for ES:
        consumeWhitespaceComment(foundNonSpaceTab);
        if (foundNonSpaceTab)
            versionNotFirst = true;

        // "#"
        if (get() != '#') {
            versionNotFirst = true;
            continue;
        }

        // whitespace
        do {
            c = get();
        } while (c == ' ' || c == '\t');

        // "version"
        if (    c != 'v' ||
            get() != 'e' ||
            get() != 'r' ||
            get() != 's' ||
            get() != 'i' ||
            get() != 'o' ||
            get() != 'n') {
            versionNotFirst = true;
            continue;
        }

        // whitespace
        do {
            c = get();
        } while (c == ' ' || c == '\t');

        // version number
        while (c >= '0' && c <= '9') {
            version = 10 * version + (c - '0');
            c = get();
        }
        if (version == 0) {
            versionNotFirst = true;
            continue;
        }

        // whitespace
        while (c == ' ' || c == '\t')
            c = get();

        // profile
        const int maxProfileLength = 13;  // not including any 0
        char profileString[maxProfileLength];
        int profileLength;
        for (profileLength = 0; profileLength < maxProfileLength; ++profileLength) {
            if (c == EndOfInput || c == ' ' || c == '\t' || c == '\n' || c == '\r')
                break;
            profileString[profileLength] = (char)c;
            c = get();
        }
        if (c != EndOfInput && c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            versionNotFirst = true;
            continue;
        }

        if (profileLength == 2 && strncmp(profileString, "es", profileLength) == 0)
            profile = EEsProfile;
        else if (profileLength == 4 && strncmp(profileString, "core", profileLength) == 0)
            profile = ECoreProfile;
        else if (profileLength == 13 && strncmp(profileString, "compatibility", profileLength) == 0)
            profile = ECompatibilityProfile;

        return versionNotFirst;
    } while (true);
}

// Fill this in when doing glslang-level scanning, to hand back to the parser.
class TParserToken {
public:
    explicit TParserToken(YYSTYPE& b) : sType(b) { }

    YYSTYPE& sType;
protected:
    TParserToken(TParserToken&);
    TParserToken& operator=(TParserToken&);
};

} // end namespace glslang

// This is the function the glslang parser (i.e., bison) calls to get its next token
int yylex(YYSTYPE* glslangTokenDesc, glslang::TParseContext& parseContext)
{
    glslang::TParserToken token(*glslangTokenDesc);

    return parseContext.getScanContext()->tokenize(parseContext.getPpContext(), token);
}

namespace {

struct str_eq
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        return strcmp(lhs, rhs) == 0;
    }
};

struct str_hash
{
    size_t operator()(const char* str) const
    {
        // djb2
        unsigned long hash = 5381;
        int c;

        while ((c = *str++) != 0)
            hash = ((hash << 5) + hash) + c;

        return hash;
    }
};

// A single global usable by all threads, by all versions, by all languages.
// After a single process-level initialization, this is read only and thread safe
std::unordered_map<const char*, int, str_hash, str_eq>* KeywordMap = nullptr;
std::unordered_set<const char*, str_hash, str_eq>* ReservedSet = nullptr;

};

namespace glslang {

void TScanContext::fillInKeywordMap()
{
    if (KeywordMap != nullptr) {
        // this is really an error, as this should called only once per process
        // but, the only risk is if two threads called simultaneously
        return;
    }
    KeywordMap = new std::unordered_map<const char*, int, str_hash, str_eq>;

    (*KeywordMap)["const"] =                   CONST;
    (*KeywordMap)["uniform"] =                 UNIFORM;
    (*KeywordMap)["in"] =                      IN;
    (*KeywordMap)["out"] =                     OUT;
    (*KeywordMap)["inout"] =                   INOUT;
    (*KeywordMap)["struct"] =                  STRUCT;
    (*KeywordMap)["break"] =                   BREAK;
    (*KeywordMap)["continue"] =                CONTINUE;
    (*KeywordMap)["do"] =                      DO;
    (*KeywordMap)["for"] =                     FOR;
    (*KeywordMap)["while"] =                   WHILE;
    (*KeywordMap)["switch"] =                  SWITCH;
    (*KeywordMap)["case"] =                    CASE;
    (*KeywordMap)["default"] =                 DEFAULT;
    (*KeywordMap)["if"] =                      IF;
    (*KeywordMap)["else"] =                    ELSE;
    (*KeywordMap)["discard"] =                 DISCARD;
    (*KeywordMap)["return"] =                  RETURN;
    (*KeywordMap)["void"] =                    VOID;
    (*KeywordMap)["bool"] =                    BOOL;
    (*KeywordMap)["float"] =                   FLOAT;
    (*KeywordMap)["int"] =                     INT;
    (*KeywordMap)["bvec2"] =                   BVEC2;
    (*KeywordMap)["bvec3"] =                   BVEC3;
    (*KeywordMap)["bvec4"] =                   BVEC4;
    (*KeywordMap)["vec2"] =                    VEC2;
    (*KeywordMap)["vec3"] =                    VEC3;
    (*KeywordMap)["vec4"] =                    VEC4;
    (*KeywordMap)["ivec2"] =                   IVEC2;
    (*KeywordMap)["ivec3"] =                   IVEC3;
    (*KeywordMap)["ivec4"] =                   IVEC4;
    (*KeywordMap)["mat2"] =                    MAT2;
    (*KeywordMap)["mat3"] =                    MAT3;
    (*KeywordMap)["mat4"] =                    MAT4;
    (*KeywordMap)["true"] =                    BOOLCONSTANT;
    (*KeywordMap)["false"] =                   BOOLCONSTANT;
    (*KeywordMap)["attribute"] =               ATTRIBUTE;
    (*KeywordMap)["varying"] =                 VARYING;
    (*KeywordMap)["buffer"] =                  BUFFER;
    (*KeywordMap)["coherent"] =                COHERENT;
    (*KeywordMap)["restrict"] =                RESTRICT;
    (*KeywordMap)["readonly"] =                READONLY;
    (*KeywordMap)["writeonly"] =               WRITEONLY;
    (*KeywordMap)["atomic_uint"] =             ATOMIC_UINT;
    (*KeywordMap)["volatile"] =                VOLATILE;
    (*KeywordMap)["layout"] =                  LAYOUT;
    (*KeywordMap)["shared"] =                  SHARED;
    (*KeywordMap)["patch"] =                   PATCH;
    (*KeywordMap)["sample"] =                  SAMPLE;
    (*KeywordMap)["subroutine"] =              SUBROUTINE;
    (*KeywordMap)["highp"] =                   HIGH_PRECISION;
    (*KeywordMap)["mediump"] =                 MEDIUM_PRECISION;
    (*KeywordMap)["lowp"] =                    LOW_PRECISION;
    (*KeywordMap)["precision"] =               PRECISION;
    (*KeywordMap)["mat2x2"] =                  MAT2X2;
    (*KeywordMap)["mat2x3"] =                  MAT2X3;
    (*KeywordMap)["mat2x4"] =                  MAT2X4;
    (*KeywordMap)["mat3x2"] =                  MAT3X2;
    (*KeywordMap)["mat3x3"] =                  MAT3X3;
    (*KeywordMap)["mat3x4"] =                  MAT3X4;
    (*KeywordMap)["mat4x2"] =                  MAT4X2;
    (*KeywordMap)["mat4x3"] =                  MAT4X3;
    (*KeywordMap)["mat4x4"] =                  MAT4X4;
    (*KeywordMap)["dmat2"] =                   DMAT2;
    (*KeywordMap)["dmat3"] =                   DMAT3;
    (*KeywordMap)["dmat4"] =                   DMAT4;
    (*KeywordMap)["dmat2x2"] =                 DMAT2X2;
    (*KeywordMap)["dmat2x3"] =                 DMAT2X3;
    (*KeywordMap)["dmat2x4"] =                 DMAT2X4;
    (*KeywordMap)["dmat3x2"] =                 DMAT3X2;
    (*KeywordMap)["dmat3x3"] =                 DMAT3X3;
    (*KeywordMap)["dmat3x4"] =                 DMAT3X4;
    (*KeywordMap)["dmat4x2"] =                 DMAT4X2;
    (*KeywordMap)["dmat4x3"] =                 DMAT4X3;
    (*KeywordMap)["dmat4x4"] =                 DMAT4X4;
    (*KeywordMap)["image1D"] =                 IMAGE1D;
    (*KeywordMap)["iimage1D"] =                IIMAGE1D;
    (*KeywordMap)["uimage1D"] =                UIMAGE1D;
    (*KeywordMap)["image2D"] =                 IMAGE2D;
    (*KeywordMap)["iimage2D"] =                IIMAGE2D;
    (*KeywordMap)["uimage2D"] =                UIMAGE2D;
    (*KeywordMap)["image3D"] =                 IMAGE3D;
    (*KeywordMap)["iimage3D"] =                IIMAGE3D;
    (*KeywordMap)["uimage3D"] =                UIMAGE3D;
    (*KeywordMap)["image2DRect"] =             IMAGE2DRECT;
    (*KeywordMap)["iimage2DRect"] =            IIMAGE2DRECT;
    (*KeywordMap)["uimage2DRect"] =            UIMAGE2DRECT;
    (*KeywordMap)["imageCube"] =               IMAGECUBE;
    (*KeywordMap)["iimageCube"] =              IIMAGECUBE;
    (*KeywordMap)["uimageCube"] =              UIMAGECUBE;
    (*KeywordMap)["imageBuffer"] =             IMAGEBUFFER;
    (*KeywordMap)["iimageBuffer"] =            IIMAGEBUFFER;
    (*KeywordMap)["uimageBuffer"] =            UIMAGEBUFFER;
    (*KeywordMap)["image1DArray"] =            IMAGE1DARRAY;
    (*KeywordMap)["iimage1DArray"] =           IIMAGE1DARRAY;
    (*KeywordMap)["uimage1DArray"] =           UIMAGE1DARRAY;
    (*KeywordMap)["image2DArray"] =            IMAGE2DARRAY;
    (*KeywordMap)["iimage2DArray"] =           IIMAGE2DARRAY;
    (*KeywordMap)["uimage2DArray"] =           UIMAGE2DARRAY;
    (*KeywordMap)["imageCubeArray"] =          IMAGECUBEARRAY;
    (*KeywordMap)["iimageCubeArray"] =         IIMAGECUBEARRAY;
    (*KeywordMap)["uimageCubeArray"] =         UIMAGECUBEARRAY;
    (*KeywordMap)["image2DMS"] =               IMAGE2DMS;
    (*KeywordMap)["iimage2DMS"] =              IIMAGE2DMS;
    (*KeywordMap)["uimage2DMS"] =              UIMAGE2DMS;
    (*KeywordMap)["image2DMSArray"] =          IMAGE2DMSARRAY;
    (*KeywordMap)["iimage2DMSArray"] =         IIMAGE2DMSARRAY;
    (*KeywordMap)["uimage2DMSArray"] =         UIMAGE2DMSARRAY;
    (*KeywordMap)["double"] =                  DOUBLE;
    (*KeywordMap)["dvec2"] =                   DVEC2;
    (*KeywordMap)["dvec3"] =                   DVEC3;
    (*KeywordMap)["dvec4"] =                   DVEC4;
    (*KeywordMap)["uint"] =                    UINT;
    (*KeywordMap)["uvec2"] =                   UVEC2;
    (*KeywordMap)["uvec3"] =                   UVEC3;
    (*KeywordMap)["uvec4"] =                   UVEC4;

    (*KeywordMap)["int64_t"] =                 INT64_T;
    (*KeywordMap)["uint64_t"] =                UINT64_T;
    (*KeywordMap)["i64vec2"] =                 I64VEC2;
    (*KeywordMap)["i64vec3"] =                 I64VEC3;
    (*KeywordMap)["i64vec4"] =                 I64VEC4;
    (*KeywordMap)["u64vec2"] =                 U64VEC2;
    (*KeywordMap)["u64vec3"] =                 U64VEC3;
    (*KeywordMap)["u64vec4"] =                 U64VEC4;

#ifdef AMD_EXTENSIONS
    (*KeywordMap)["int16_t"] =                 INT16_T;
    (*KeywordMap)["uint16_t"] =                UINT16_T;
    (*KeywordMap)["i16vec2"] =                 I16VEC2;
    (*KeywordMap)["i16vec3"] =                 I16VEC3;
    (*KeywordMap)["i16vec4"] =                 I16VEC4;
    (*KeywordMap)["u16vec2"] =                 U16VEC2;
    (*KeywordMap)["u16vec3"] =                 U16VEC3;
    (*KeywordMap)["u16vec4"] =                 U16VEC4;

    (*KeywordMap)["float16_t"] =               FLOAT16_T;
    (*KeywordMap)["f16vec2"] =                 F16VEC2;
    (*KeywordMap)["f16vec3"] =                 F16VEC3;
    (*KeywordMap)["f16vec4"] =                 F16VEC4;
    (*KeywordMap)["f16mat2"] =                 F16MAT2;
    (*KeywordMap)["f16mat3"] =                 F16MAT3;
    (*KeywordMap)["f16mat4"] =                 F16MAT4;
    (*KeywordMap)["f16mat2x2"] =               F16MAT2X2;
    (*KeywordMap)["f16mat2x3"] =               F16MAT2X3;
    (*KeywordMap)["f16mat2x4"] =               F16MAT2X4;
    (*KeywordMap)["f16mat3x2"] =               F16MAT3X2;
    (*KeywordMap)["f16mat3x3"] =               F16MAT3X3;
    (*KeywordMap)["f16mat3x4"] =               F16MAT3X4;
    (*KeywordMap)["f16mat4x2"] =               F16MAT4X2;
    (*KeywordMap)["f16mat4x3"] =               F16MAT4X3;
    (*KeywordMap)["f16mat4x4"] =               F16MAT4X4;
#endif

    (*KeywordMap)["sampler2D"] =               SAMPLER2D;
    (*KeywordMap)["samplerCube"] =             SAMPLERCUBE;
    (*KeywordMap)["samplerCubeArray"] =        SAMPLERCUBEARRAY;
    (*KeywordMap)["samplerCubeArrayShadow"] =  SAMPLERCUBEARRAYSHADOW;
    (*KeywordMap)["isamplerCubeArray"] =       ISAMPLERCUBEARRAY;
    (*KeywordMap)["usamplerCubeArray"] =       USAMPLERCUBEARRAY;
    (*KeywordMap)["sampler1DArrayShadow"] =    SAMPLER1DARRAYSHADOW;
    (*KeywordMap)["isampler1DArray"] =         ISAMPLER1DARRAY;
    (*KeywordMap)["usampler1D"] =              USAMPLER1D;
    (*KeywordMap)["isampler1D"] =              ISAMPLER1D;
    (*KeywordMap)["usampler1DArray"] =         USAMPLER1DARRAY;
    (*KeywordMap)["samplerBuffer"] =           SAMPLERBUFFER;
    (*KeywordMap)["samplerCubeShadow"] =       SAMPLERCUBESHADOW;
    (*KeywordMap)["sampler2DArray"] =          SAMPLER2DARRAY;
    (*KeywordMap)["sampler2DArrayShadow"] =    SAMPLER2DARRAYSHADOW;
    (*KeywordMap)["isampler2D"] =              ISAMPLER2D;
    (*KeywordMap)["isampler3D"] =              ISAMPLER3D;
    (*KeywordMap)["isamplerCube"] =            ISAMPLERCUBE;
    (*KeywordMap)["isampler2DArray"] =         ISAMPLER2DARRAY;
    (*KeywordMap)["usampler2D"] =              USAMPLER2D;
    (*KeywordMap)["usampler3D"] =              USAMPLER3D;
    (*KeywordMap)["usamplerCube"] =            USAMPLERCUBE;
    (*KeywordMap)["usampler2DArray"] =         USAMPLER2DARRAY;
    (*KeywordMap)["isampler2DRect"] =          ISAMPLER2DRECT;
    (*KeywordMap)["usampler2DRect"] =          USAMPLER2DRECT;
    (*KeywordMap)["isamplerBuffer"] =          ISAMPLERBUFFER;
    (*KeywordMap)["usamplerBuffer"] =          USAMPLERBUFFER;
    (*KeywordMap)["sampler2DMS"] =             SAMPLER2DMS;
    (*KeywordMap)["isampler2DMS"] =            ISAMPLER2DMS;
    (*KeywordMap)["usampler2DMS"] =            USAMPLER2DMS;
    (*KeywordMap)["sampler2DMSArray"] =        SAMPLER2DMSARRAY;
    (*KeywordMap)["isampler2DMSArray"] =       ISAMPLER2DMSARRAY;
    (*KeywordMap)["usampler2DMSArray"] =       USAMPLER2DMSARRAY;
    (*KeywordMap)["sampler1D"] =               SAMPLER1D;
    (*KeywordMap)["sampler1DShadow"] =         SAMPLER1DSHADOW;
    (*KeywordMap)["sampler3D"] =               SAMPLER3D;
    (*KeywordMap)["sampler2DShadow"] =         SAMPLER2DSHADOW;
    (*KeywordMap)["sampler2DRect"] =           SAMPLER2DRECT;
    (*KeywordMap)["sampler2DRectShadow"] =     SAMPLER2DRECTSHADOW;
    (*KeywordMap)["sampler1DArray"] =          SAMPLER1DARRAY;

    (*KeywordMap)["samplerExternalOES"] =      SAMPLEREXTERNALOES; // GL_OES_EGL_image_external

    (*KeywordMap)["sampler"] =                 SAMPLER;
    (*KeywordMap)["samplerShadow"] =           SAMPLERSHADOW;

    (*KeywordMap)["texture2D"] =               TEXTURE2D;
    (*KeywordMap)["textureCube"] =             TEXTURECUBE;
    (*KeywordMap)["textureCubeArray"] =        TEXTURECUBEARRAY;
    (*KeywordMap)["itextureCubeArray"] =       ITEXTURECUBEARRAY;
    (*KeywordMap)["utextureCubeArray"] =       UTEXTURECUBEARRAY;
    (*KeywordMap)["itexture1DArray"] =         ITEXTURE1DARRAY;
    (*KeywordMap)["utexture1D"] =              UTEXTURE1D;
    (*KeywordMap)["itexture1D"] =              ITEXTURE1D;
    (*KeywordMap)["utexture1DArray"] =         UTEXTURE1DARRAY;
    (*KeywordMap)["textureBuffer"] =           TEXTUREBUFFER;
    (*KeywordMap)["texture2DArray"] =          TEXTURE2DARRAY;
    (*KeywordMap)["itexture2D"] =              ITEXTURE2D;
    (*KeywordMap)["itexture3D"] =              ITEXTURE3D;
    (*KeywordMap)["itextureCube"] =            ITEXTURECUBE;
    (*KeywordMap)["itexture2DArray"] =         ITEXTURE2DARRAY;
    (*KeywordMap)["utexture2D"] =              UTEXTURE2D;
    (*KeywordMap)["utexture3D"] =              UTEXTURE3D;
    (*KeywordMap)["utextureCube"] =            UTEXTURECUBE;
    (*KeywordMap)["utexture2DArray"] =         UTEXTURE2DARRAY;
    (*KeywordMap)["itexture2DRect"] =          ITEXTURE2DRECT;
    (*KeywordMap)["utexture2DRect"] =          UTEXTURE2DRECT;
    (*KeywordMap)["itextureBuffer"] =          ITEXTUREBUFFER;
    (*KeywordMap)["utextureBuffer"] =          UTEXTUREBUFFER;
    (*KeywordMap)["texture2DMS"] =             TEXTURE2DMS;
    (*KeywordMap)["itexture2DMS"] =            ITEXTURE2DMS;
    (*KeywordMap)["utexture2DMS"] =            UTEXTURE2DMS;
    (*KeywordMap)["texture2DMSArray"] =        TEXTURE2DMSARRAY;
    (*KeywordMap)["itexture2DMSArray"] =       ITEXTURE2DMSARRAY;
    (*KeywordMap)["utexture2DMSArray"] =       UTEXTURE2DMSARRAY;
    (*KeywordMap)["texture1D"] =               TEXTURE1D;
    (*KeywordMap)["texture3D"] =               TEXTURE3D;
    (*KeywordMap)["texture2DRect"] =           TEXTURE2DRECT;
    (*KeywordMap)["texture1DArray"] =          TEXTURE1DARRAY;

    (*KeywordMap)["subpassInput"] =            SUBPASSINPUT;
    (*KeywordMap)["subpassInputMS"] =          SUBPASSINPUTMS;
    (*KeywordMap)["isubpassInput"] =           ISUBPASSINPUT;
    (*KeywordMap)["isubpassInputMS"] =         ISUBPASSINPUTMS;
    (*KeywordMap)["usubpassInput"] =           USUBPASSINPUT;
    (*KeywordMap)["usubpassInputMS"] =         USUBPASSINPUTMS;

    (*KeywordMap)["noperspective"] =           NOPERSPECTIVE;
    (*KeywordMap)["smooth"] =                  SMOOTH;
    (*KeywordMap)["flat"] =                    FLAT;
#ifdef AMD_EXTENSIONS
    (*KeywordMap)["__explicitInterpAMD"] =     __EXPLICITINTERPAMD;
#endif
    (*KeywordMap)["centroid"] =                CENTROID;
    (*KeywordMap)["precise"] =                 PRECISE;
    (*KeywordMap)["invariant"] =               INVARIANT;
    (*KeywordMap)["packed"] =                  PACKED;
    (*KeywordMap)["resource"] =                RESOURCE;
    (*KeywordMap)["superp"] =                  SUPERP;

    ReservedSet = new std::unordered_set<const char*, str_hash, str_eq>;

    ReservedSet->insert("common");
    ReservedSet->insert("partition");
    ReservedSet->insert("active");
    ReservedSet->insert("asm");
    ReservedSet->insert("class");
    ReservedSet->insert("union");
    ReservedSet->insert("enum");
    ReservedSet->insert("typedef");
    ReservedSet->insert("template");
    ReservedSet->insert("this");
    ReservedSet->insert("goto");
    ReservedSet->insert("inline");
    ReservedSet->insert("noinline");
    ReservedSet->insert("public");
    ReservedSet->insert("static");
    ReservedSet->insert("extern");
    ReservedSet->insert("external");
    ReservedSet->insert("interface");
    ReservedSet->insert("long");
    ReservedSet->insert("short");
    ReservedSet->insert("half");
    ReservedSet->insert("fixed");
    ReservedSet->insert("unsigned");
    ReservedSet->insert("input");
    ReservedSet->insert("output");
    ReservedSet->insert("hvec2");
    ReservedSet->insert("hvec3");
    ReservedSet->insert("hvec4");
    ReservedSet->insert("fvec2");
    ReservedSet->insert("fvec3");
    ReservedSet->insert("fvec4");
    ReservedSet->insert("sampler3DRect");
    ReservedSet->insert("filter");
    ReservedSet->insert("sizeof");
    ReservedSet->insert("cast");
    ReservedSet->insert("namespace");
    ReservedSet->insert("using");
}

void TScanContext::deleteKeywordMap()
{
    delete KeywordMap;
    KeywordMap = nullptr;
    delete ReservedSet;
    ReservedSet = nullptr;
}

// Called by yylex to get the next token.
// Returning 0 implies end of input.
int TScanContext::tokenize(TPpContext* pp, TParserToken& token)
{
    do {
        parserToken = &token;
        TPpToken ppToken;
        int token = pp->tokenize(ppToken);
        if (token == EndOfInput)
            return 0;

        tokenText = ppToken.name;
        loc = ppToken.loc;
        parserToken->sType.lex.loc = loc;
        switch (token) {
        case ';':  afterType = false;   return SEMICOLON;
        case ',':  afterType = false;   return COMMA;
        case ':':                       return COLON;
        case '=':  afterType = false;   return EQUAL;
        case '(':  afterType = false;   return LEFT_PAREN;
        case ')':  afterType = false;   return RIGHT_PAREN;
        case '.':  field = true;        return DOT;
        case '!':                       return BANG;
        case '-':                       return DASH;
        case '~':                       return TILDE;
        case '+':                       return PLUS;
        case '*':                       return STAR;
        case '/':                       return SLASH;
        case '%':                       return PERCENT;
        case '<':                       return LEFT_ANGLE;
        case '>':                       return RIGHT_ANGLE;
        case '|':                       return VERTICAL_BAR;
        case '^':                       return CARET;
        case '&':                       return AMPERSAND;
        case '?':                       return QUESTION;
        case '[':                       return LEFT_BRACKET;
        case ']':                       return RIGHT_BRACKET;
        case '{':                       return LEFT_BRACE;
        case '}':                       return RIGHT_BRACE;
        case '\\':
            parseContext.error(loc, "illegal use of escape character", "\\", "");
            break;

        case PPAtomAddAssign:          return ADD_ASSIGN;
        case PPAtomSubAssign:          return SUB_ASSIGN;
        case PPAtomMulAssign:          return MUL_ASSIGN;
        case PPAtomDivAssign:          return DIV_ASSIGN;
        case PPAtomModAssign:          return MOD_ASSIGN;

        case PpAtomRight:              return RIGHT_OP;
        case PpAtomLeft:               return LEFT_OP;

        case PpAtomRightAssign:        return RIGHT_ASSIGN;
        case PpAtomLeftAssign:         return LEFT_ASSIGN;
        case PpAtomAndAssign:          return AND_ASSIGN;
        case PpAtomOrAssign:           return OR_ASSIGN;
        case PpAtomXorAssign:          return XOR_ASSIGN;

        case PpAtomAnd:                return AND_OP;
        case PpAtomOr:                 return OR_OP;
        case PpAtomXor:                return XOR_OP;

        case PpAtomEQ:                 return EQ_OP;
        case PpAtomGE:                 return GE_OP;
        case PpAtomNE:                 return NE_OP;
        case PpAtomLE:                 return LE_OP;

        case PpAtomDecrement:          return DEC_OP;
        case PpAtomIncrement:          return INC_OP;

        case PpAtomColonColon:
            parseContext.error(loc, "not supported", "::", "");
            break;

        case PpAtomConstInt:           parserToken->sType.lex.i   = ppToken.ival;       return INTCONSTANT;
        case PpAtomConstUint:          parserToken->sType.lex.i   = ppToken.ival;       return UINTCONSTANT;
        case PpAtomConstInt64:         parserToken->sType.lex.i64 = ppToken.i64val;     return INT64CONSTANT;
        case PpAtomConstUint64:        parserToken->sType.lex.i64 = ppToken.i64val;     return UINT64CONSTANT;
#ifdef AMD_EXTENSIONS
        case PpAtomConstInt16:         parserToken->sType.lex.i   = ppToken.ival;       return INT16CONSTANT;
        case PpAtomConstUint16:        parserToken->sType.lex.i   = ppToken.ival;       return UINT16CONSTANT;
#endif
        case PpAtomConstFloat:         parserToken->sType.lex.d   = ppToken.dval;       return FLOATCONSTANT;
        case PpAtomConstDouble:        parserToken->sType.lex.d   = ppToken.dval;       return DOUBLECONSTANT;
#ifdef AMD_EXTENSIONS
        case PpAtomConstFloat16:       parserToken->sType.lex.d   = ppToken.dval;       return FLOAT16CONSTANT;
#endif
        case PpAtomIdentifier:
        {
            int token = tokenizeIdentifier();
            field = false;
            return token;
        }

        case EndOfInput:               return 0;

        default:
            char buf[2];
            buf[0] = (char)token;
            buf[1] = 0;
            parseContext.error(loc, "unexpected token", buf, "");
            break;
        }
    } while (true);
}

int TScanContext::tokenizeIdentifier()
{
    if (ReservedSet->find(tokenText) != ReservedSet->end())
        return reservedWord();

    auto it = KeywordMap->find(tokenText);
    if (it == KeywordMap->end()) {
        // Should have an identifier of some sort
        return identifierOrType();
    }
    keyword = it->second;

    switch (keyword) {
    case CONST:
    case UNIFORM:
    case IN:
    case OUT:
    case INOUT:
    case STRUCT:
    case BREAK:
    case CONTINUE:
    case DO:
    case FOR:
    case WHILE:
    case IF:
    case ELSE:
    case DISCARD:
    case RETURN:
    case CASE:
        return keyword;

    case SWITCH:
    case DEFAULT:
        if ((parseContext.profile == EEsProfile && parseContext.version < 300) ||
            (parseContext.profile != EEsProfile && parseContext.version < 130))
            reservedWord();
        return keyword;

    case VOID:
    case BOOL:
    case FLOAT:
    case INT:
    case BVEC2:
    case BVEC3:
    case BVEC4:
    case VEC2:
    case VEC3:
    case VEC4:
    case IVEC2:
    case IVEC3:
    case IVEC4:
    case MAT2:
    case MAT3:
    case MAT4:
    case SAMPLER2D:
    case SAMPLERCUBE:
        afterType = true;
        return keyword;

    case BOOLCONSTANT:
        if (strcmp("true", tokenText) == 0)
            parserToken->sType.lex.b = true;
        else
            parserToken->sType.lex.b = false;
        return keyword;

    case ATTRIBUTE:
    case VARYING:
        if (parseContext.profile == EEsProfile && parseContext.version >= 300)
            reservedWord();
        return keyword;

    case BUFFER:
        if ((parseContext.profile == EEsProfile && parseContext.version < 310) ||
            (parseContext.profile != EEsProfile && parseContext.version < 430))
            return identifierOrType();
        return keyword;

    case ATOMIC_UINT:
        if ((parseContext.profile == EEsProfile && parseContext.version >= 310) ||
            parseContext.extensionTurnedOn(E_GL_ARB_shader_atomic_counters))
            return keyword;
        return es30ReservedFromGLSL(420);

    case COHERENT:
    case RESTRICT:
    case READONLY:
    case WRITEONLY:
        if (parseContext.profile == EEsProfile && parseContext.version >= 310)
            return keyword;
        return es30ReservedFromGLSL(parseContext.extensionTurnedOn(E_GL_ARB_shader_image_load_store) ? 130 : 420);

    case VOLATILE:
        if (parseContext.profile == EEsProfile && parseContext.version >= 310)
            return keyword;
        if (! parseContext.symbolTable.atBuiltInLevel() && (parseContext.profile == EEsProfile || (parseContext.version < 420 && ! parseContext.extensionTurnedOn(E_GL_ARB_shader_image_load_store))))
            reservedWord();
        return keyword;

    case LAYOUT:
    {
        const int numLayoutExts = 2;
        const char* layoutExts[numLayoutExts] = { E_GL_ARB_shading_language_420pack,
                                                  E_GL_ARB_explicit_attrib_location };
        if ((parseContext.profile == EEsProfile && parseContext.version < 300) ||
            (parseContext.profile != EEsProfile && parseContext.version < 140 &&
            ! parseContext.extensionsTurnedOn(numLayoutExts, layoutExts)))
            return identifierOrType();
        return keyword;
    }
    case SHARED:
        if ((parseContext.profile == EEsProfile && parseContext.version < 300) ||
            (parseContext.profile != EEsProfile && parseContext.version < 140))
            return identifierOrType();
        return keyword;

    case PATCH:
        if (parseContext.symbolTable.atBuiltInLevel() ||
            (parseContext.profile == EEsProfile &&
             (parseContext.version >= 320 || 
              parseContext.extensionsTurnedOn(Num_AEP_tessellation_shader, AEP_tessellation_shader))) ||
            (parseContext.profile != EEsProfile && parseContext.extensionTurnedOn(E_GL_ARB_tessellation_shader)))
            return keyword;

        return es30ReservedFromGLSL(400);

    case SAMPLE:
        if ((parseContext.profile == EEsProfile && parseContext.version >= 320) ||
            parseContext.extensionsTurnedOn(1, &E_GL_OES_shader_multisample_interpolation))
            return keyword;
        return es30ReservedFromGLSL(400);

    case SUBROUTINE:
        return es30ReservedFromGLSL(400);

    case HIGH_PRECISION:
    case MEDIUM_PRECISION:
    case LOW_PRECISION:
    case PRECISION:
        return precisionKeyword();

    case MAT2X2:
    case MAT2X3:
    case MAT2X4:
    case MAT3X2:
    case MAT3X3:
    case MAT3X4:
    case MAT4X2:
    case MAT4X3:
    case MAT4X4:
        return matNxM();

    case DMAT2:
    case DMAT3:
    case DMAT4:
    case DMAT2X2:
    case DMAT2X3:
    case DMAT2X4:
    case DMAT3X2:
    case DMAT3X3:
    case DMAT3X4:
    case DMAT4X2:
    case DMAT4X3:
    case DMAT4X4:
        return dMat();

    case IMAGE1D:
    case IIMAGE1D:
    case UIMAGE1D:
    case IMAGE1DARRAY:
    case IIMAGE1DARRAY:
    case UIMAGE1DARRAY:
    case IMAGE2DRECT:
    case IIMAGE2DRECT:
    case UIMAGE2DRECT:
        afterType = true;
        return firstGenerationImage(false);

    case IMAGEBUFFER:
    case IIMAGEBUFFER:
    case UIMAGEBUFFER:
        afterType = true;
        if ((parseContext.profile == EEsProfile && parseContext.version >= 320) ||
            parseContext.extensionsTurnedOn(Num_AEP_texture_buffer, AEP_texture_buffer))
            return keyword;
        return firstGenerationImage(false);

    case IMAGE2D:
    case IIMAGE2D:
    case UIMAGE2D:
    case IMAGE3D:
    case IIMAGE3D:
    case UIMAGE3D:
    case IMAGECUBE:
    case IIMAGECUBE:
    case UIMAGECUBE:
    case IMAGE2DARRAY:
    case IIMAGE2DARRAY:
    case UIMAGE2DARRAY:
        afterType = true;
        return firstGenerationImage(true);

    case IMAGECUBEARRAY:
    case IIMAGECUBEARRAY:
    case UIMAGECUBEARRAY:
        afterType = true;
        if ((parseContext.profile == EEsProfile && parseContext.version >= 320) ||
            parseContext.extensionsTurnedOn(Num_AEP_texture_cube_map_array, AEP_texture_cube_map_array))
            return keyword;
        return secondGenerationImage();

    case IMAGE2DMS:
    case IIMAGE2DMS:
    case UIMAGE2DMS:
    case IMAGE2DMSARRAY:
    case IIMAGE2DMSARRAY:
    case UIMAGE2DMSARRAY:
        afterType = true;
        return secondGenerationImage();

    case DOUBLE:
    case DVEC2:
    case DVEC3:
    case DVEC4:
        afterType = true;
        if (parseContext.profile == EEsProfile || parseContext.version < 400)
            reservedWord();
        return keyword;

    case INT64_T:
    case UINT64_T:
    case I64VEC2:
    case I64VEC3:
    case I64VEC4:
    case U64VEC2:
    case U64VEC3:
    case U64VEC4:
        afterType = true;
        if (parseContext.symbolTable.atBuiltInLevel() ||
            (parseContext.extensionTurnedOn(E_GL_ARB_gpu_shader_int64) &&
             parseContext.profile != EEsProfile && parseContext.version >= 450))
            return keyword;
        return identifierOrType();

#ifdef AMD_EXTENSIONS
    case INT16_T:
    case UINT16_T:
    case I16VEC2:
    case I16VEC3:
    case I16VEC4:
    case U16VEC2:
    case U16VEC3:
    case U16VEC4:
        afterType = true;
        if (parseContext.symbolTable.atBuiltInLevel() ||
            (parseContext.extensionTurnedOn(E_GL_AMD_gpu_shader_int16) &&
             parseContext.profile != EEsProfile && parseContext.version >= 450))
            return keyword;
        return identifierOrType();

    case FLOAT16_T:
    case F16VEC2:
    case F16VEC3:
    case F16VEC4:
    case F16MAT2:
    case F16MAT3:
    case F16MAT4:
    case F16MAT2X2:
    case F16MAT2X3:
    case F16MAT2X4:
    case F16MAT3X2:
    case F16MAT3X3:
    case F16MAT3X4:
    case F16MAT4X2:
    case F16MAT4X3:
    case F16MAT4X4:
        afterType = true;
        if (parseContext.symbolTable.atBuiltInLevel() ||
            (parseContext.extensionTurnedOn(E_GL_AMD_gpu_shader_half_float) &&
             parseContext.profile != EEsProfile && parseContext.version >= 450))
            return keyword;
        return identifierOrType();
#endif

    case SAMPLERCUBEARRAY:
    case SAMPLERCUBEARRAYSHADOW:
    case ISAMPLERCUBEARRAY:
    case USAMPLERCUBEARRAY:
        afterType = true;
        if ((parseContext.profile == EEsProfile && parseContext.version >= 320) ||
            parseContext.extensionsTurnedOn(Num_AEP_texture_cube_map_array, AEP_texture_cube_map_array))
            return keyword;
        if (parseContext.profile == EEsProfile || (parseContext.version < 400 && ! parseContext.extensionTurnedOn(E_GL_ARB_texture_cube_map_array)))
            reservedWord();
        return keyword;

    case ISAMPLER1D:
    case ISAMPLER1DARRAY:
    case SAMPLER1DARRAYSHADOW:
    case USAMPLER1D:
    case USAMPLER1DARRAY:
        afterType = true;
        return es30ReservedFromGLSL(130);

    case UINT:
    case UVEC2:
    case UVEC3:
    case UVEC4:
    case SAMPLERCUBESHADOW:
    case SAMPLER2DARRAY:
    case SAMPLER2DARRAYSHADOW:
    case ISAMPLER2D:
    case ISAMPLER3D:
    case ISAMPLERCUBE:
    case ISAMPLER2DARRAY:
    case USAMPLER2D:
    case USAMPLER3D:
    case USAMPLERCUBE:
    case USAMPLER2DARRAY:
        afterType = true;
        return nonreservedKeyword(300, 130);

    case ISAMPLER2DRECT:
    case USAMPLER2DRECT:
        afterType = true;
        return es30ReservedFromGLSL(140);

    case SAMPLERBUFFER:
        afterType = true;
        if ((parseContext.profile == EEsProfile && parseContext.version >= 320) ||
            parseContext.extensionsTurnedOn(Num_AEP_texture_buffer, AEP_texture_buffer))
            return keyword;
        return es30ReservedFromGLSL(130);

    case ISAMPLERBUFFER:
    case USAMPLERBUFFER:
        afterType = true;
        if ((parseContext.profile == EEsProfile && parseContext.version >= 320) ||
            parseContext.extensionsTurnedOn(Num_AEP_texture_buffer, AEP_texture_buffer))
            return keyword;
        return es30ReservedFromGLSL(140);

    case SAMPLER2DMS:
    case ISAMPLER2DMS:
    case USAMPLER2DMS:
        afterType = true;
        if (parseContext.profile == EEsProfile && parseContext.version >= 310)
            return keyword;
        return es30ReservedFromGLSL(150);

    case SAMPLER2DMSARRAY:
    case ISAMPLER2DMSARRAY:
    case USAMPLER2DMSARRAY:
        afterType = true;
        if ((parseContext.profile == EEsProfile && parseContext.version >= 320) ||
            parseContext.extensionsTurnedOn(1, &E_GL_OES_texture_storage_multisample_2d_array))
            return keyword;
        return es30ReservedFromGLSL(150);

    case SAMPLER1D:
    case SAMPLER1DSHADOW:
        afterType = true;
        if (parseContext.profile == EEsProfile)
            reservedWord();
        return keyword;

    case SAMPLER3D:
        afterType = true;
        if (parseContext.profile == EEsProfile && parseContext.version < 300) {
            if (! parseContext.extensionTurnedOn(E_GL_OES_texture_3D))
                reservedWord();
        }
        return keyword;

    case SAMPLER2DSHADOW:
        afterType = true;
        if (parseContext.profile == EEsProfile && parseContext.version < 300) {
            if (!parseContext.extensionTurnedOn(E_GL_EXT_shadow_samplers))
                reservedWord();
        }
        return keyword;

    case SAMPLER2DRECT:
    case SAMPLER2DRECTSHADOW:
        afterType = true;
        if (parseContext.profile == EEsProfile)
            reservedWord();
        else if (parseContext.version < 140 && ! parseContext.symbolTable.atBuiltInLevel() && ! parseContext.extensionTurnedOn(E_GL_ARB_texture_rectangle)) {
            if (parseContext.relaxedErrors())
                parseContext.requireExtensions(loc, 1, &E_GL_ARB_texture_rectangle, "texture-rectangle sampler keyword");
            else
                reservedWord();
        }
        return keyword;

    case SAMPLER1DARRAY:
        afterType = true;
        if (parseContext.profile == EEsProfile && parseContext.version == 300)
            reservedWord();
        else if ((parseContext.profile == EEsProfile && parseContext.version < 300) ||
                 (parseContext.profile != EEsProfile && parseContext.version < 130))
            return identifierOrType();
        return keyword;

    case SAMPLEREXTERNALOES:
        afterType = true;
        if (parseContext.symbolTable.atBuiltInLevel() || parseContext.extensionTurnedOn(E_GL_OES_EGL_image_external))
            return keyword;
        return identifierOrType();

    case TEXTURE2D:
    case TEXTURECUBE:
    case TEXTURECUBEARRAY:
    case ITEXTURECUBEARRAY:
    case UTEXTURECUBEARRAY:
    case ITEXTURE1DARRAY:
    case UTEXTURE1D:
    case ITEXTURE1D:
    case UTEXTURE1DARRAY:
    case TEXTUREBUFFER:
    case TEXTURE2DARRAY:
    case ITEXTURE2D:
    case ITEXTURE3D:
    case ITEXTURECUBE:
    case ITEXTURE2DARRAY:
    case UTEXTURE2D:
    case UTEXTURE3D:
    case UTEXTURECUBE:
    case UTEXTURE2DARRAY:
    case ITEXTURE2DRECT:
    case UTEXTURE2DRECT:
    case ITEXTUREBUFFER:
    case UTEXTUREBUFFER:
    case TEXTURE2DMS:
    case ITEXTURE2DMS:
    case UTEXTURE2DMS:
    case TEXTURE2DMSARRAY:
    case ITEXTURE2DMSARRAY:
    case UTEXTURE2DMSARRAY:
    case TEXTURE1D:
    case TEXTURE3D:
    case TEXTURE2DRECT:
    case TEXTURE1DARRAY:
    case SAMPLER:
    case SAMPLERSHADOW:
        if (parseContext.spvVersion.vulkan >= 100)
            return keyword;
        else
            return identifierOrType();

    case SUBPASSINPUT:
    case SUBPASSINPUTMS:
    case ISUBPASSINPUT:
    case ISUBPASSINPUTMS:
    case USUBPASSINPUT:
    case USUBPASSINPUTMS:
        if (parseContext.spvVersion.vulkan >= 100)
            return keyword;
        else
            return identifierOrType();

    case NOPERSPECTIVE:
        return es30ReservedFromGLSL(130);

    case SMOOTH:
        if ((parseContext.profile == EEsProfile && parseContext.version < 300) ||
            (parseContext.profile != EEsProfile && parseContext.version < 130))
            return identifierOrType();
        return keyword;

#ifdef AMD_EXTENSIONS
    case __EXPLICITINTERPAMD:
        if (parseContext.profile != EEsProfile && parseContext.version >= 450 &&
            parseContext.extensionTurnedOn(E_GL_AMD_shader_explicit_vertex_parameter))
            return keyword;
        return identifierOrType();
#endif

    case FLAT:
        if (parseContext.profile == EEsProfile && parseContext.version < 300)
            reservedWord();
        else if (parseContext.profile != EEsProfile && parseContext.version < 130)
            return identifierOrType();
        return keyword;

    case CENTROID:
        if (parseContext.version < 120)
            return identifierOrType();
        return keyword;

    case PRECISE:
        if ((parseContext.profile == EEsProfile &&
             (parseContext.version >= 320 || parseContext.extensionsTurnedOn(Num_AEP_gpu_shader5, AEP_gpu_shader5))) ||
            (parseContext.profile != EEsProfile && parseContext.version >= 400))
            return keyword;
        if (parseContext.profile == EEsProfile && parseContext.version == 310) {
            reservedWord();
            return keyword;
        }
        return identifierOrType();

    case INVARIANT:
        if (parseContext.profile != EEsProfile && parseContext.version < 120)
            return identifierOrType();
        return keyword;

    case PACKED:
        if ((parseContext.profile == EEsProfile && parseContext.version < 300) ||
            (parseContext.profile != EEsProfile && parseContext.version < 330))
            return reservedWord();
        return identifierOrType();

    case RESOURCE:
    {
        bool reserved = (parseContext.profile == EEsProfile && parseContext.version >= 300) ||
                        (parseContext.profile != EEsProfile && parseContext.version >= 420);
        return identifierOrReserved(reserved);
    }
    case SUPERP:
    {
        bool reserved = parseContext.profile == EEsProfile || parseContext.version >= 130;
        return identifierOrReserved(reserved);
    }

    default:
        parseContext.infoSink.info.message(EPrefixInternalError, "Unknown glslang keyword", loc);
        return 0;
    }
}

int TScanContext::identifierOrType()
{
    parserToken->sType.lex.string = NewPoolTString(tokenText);
    if (field)
        return IDENTIFIER;

    parserToken->sType.lex.symbol = parseContext.symbolTable.find(*parserToken->sType.lex.string);
    if (afterType == false && parserToken->sType.lex.symbol) {
        if (const TVariable* variable = parserToken->sType.lex.symbol->getAsVariable()) {
            if (variable->isUserType()) {
                afterType = true;

                return TYPE_NAME;
            }
        }
    }

    return IDENTIFIER;
}

// Give an error for use of a reserved symbol.
// However, allow built-in declarations to use reserved words, to allow
// extension support before the extension is enabled.
int TScanContext::reservedWord()
{
    if (! parseContext.symbolTable.atBuiltInLevel())
        parseContext.error(loc, "Reserved word.", tokenText, "", "");

    return 0;
}

int TScanContext::identifierOrReserved(bool reserved)
{
    if (reserved) {
        reservedWord();

        return 0;
    }

    if (parseContext.forwardCompatible)
        parseContext.warn(loc, "using future reserved keyword", tokenText, "");

    return identifierOrType();
}

// For keywords that suddenly showed up on non-ES (not previously reserved)
// but then got reserved by ES 3.0.
int TScanContext::es30ReservedFromGLSL(int version)
{
    if (parseContext.symbolTable.atBuiltInLevel())
        return keyword;

    if ((parseContext.profile == EEsProfile && parseContext.version < 300) ||
        (parseContext.profile != EEsProfile && parseContext.version < version)) {
            if (parseContext.forwardCompatible)
                parseContext.warn(loc, "future reserved word in ES 300 and keyword in GLSL", tokenText, "");

            return identifierOrType();
    } else if (parseContext.profile == EEsProfile && parseContext.version >= 300)
        reservedWord();

    return keyword;
}

// For a keyword that was never reserved, until it suddenly
// showed up, both in an es version and a non-ES version.
int TScanContext::nonreservedKeyword(int esVersion, int nonEsVersion)
{
    if ((parseContext.profile == EEsProfile && parseContext.version < esVersion) ||
        (parseContext.profile != EEsProfile && parseContext.version < nonEsVersion)) {
        if (parseContext.forwardCompatible)
            parseContext.warn(loc, "using future keyword", tokenText, "");

        return identifierOrType();
    }

    return keyword;
}

int TScanContext::precisionKeyword()
{
    if (parseContext.profile == EEsProfile || parseContext.version >= 130)
        return keyword;

    if (parseContext.forwardCompatible)
        parseContext.warn(loc, "using ES precision qualifier keyword", tokenText, "");

    return identifierOrType();
}

int TScanContext::matNxM()
{
    afterType = true;

    if (parseContext.version > 110)
        return keyword;

    if (parseContext.forwardCompatible)
        parseContext.warn(loc, "using future non-square matrix type keyword", tokenText, "");

    return identifierOrType();
}

int TScanContext::dMat()
{
    afterType = true;

    if (parseContext.profile == EEsProfile && parseContext.version >= 300) {
        reservedWord();

        return keyword;
    }

    if (parseContext.profile != EEsProfile && parseContext.version >= 400)
        return keyword;

    if (parseContext.forwardCompatible)
        parseContext.warn(loc, "using future type keyword", tokenText, "");

    return identifierOrType();
}

int TScanContext::firstGenerationImage(bool inEs310)
{
    if (parseContext.symbolTable.atBuiltInLevel() ||
        (parseContext.profile != EEsProfile && (parseContext.version >= 420 || parseContext.extensionTurnedOn(E_GL_ARB_shader_image_load_store))) ||
        (inEs310 && parseContext.profile == EEsProfile && parseContext.version >= 310))
        return keyword;

    if ((parseContext.profile == EEsProfile && parseContext.version >= 300) ||
        (parseContext.profile != EEsProfile && parseContext.version >= 130)) {
        reservedWord();

        return keyword;
    }

    if (parseContext.forwardCompatible)
        parseContext.warn(loc, "using future type keyword", tokenText, "");

    return identifierOrType();
}

int TScanContext::secondGenerationImage()
{
    if (parseContext.profile == EEsProfile && parseContext.version >= 310) {
        reservedWord();
        return keyword;
    }

    if (parseContext.symbolTable.atBuiltInLevel() ||
        (parseContext.profile != EEsProfile &&
         (parseContext.version >= 420 || parseContext.extensionTurnedOn(E_GL_ARB_shader_image_load_store))))
        return keyword;

    if (parseContext.forwardCompatible)
        parseContext.warn(loc, "using future type keyword", tokenText, "");

    return identifierOrType();
}

} // end namespace glslang
