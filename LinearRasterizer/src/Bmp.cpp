//-------------------------------------------------------------------------------------------------
// File : Bmp.cpp
// Desc : Bitmap Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <Bmp.h>
#include <cstdio>
#include <asdxLogger.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// BMP_FILE_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
struct BMP_FILE_HEADER
{
    u16 Type;
    u32 Size;
    u16 Reserved1;
    u16 Reserved2;
    u32 OffBits;
};
#pragma pack(pop)


///////////////////////////////////////////////////////////////////////////////////////////////////
// BMP_INFO_HEADER structure
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
struct BMP_INFO_HEADER
{
    u32  Size;
    s32  Width;
    s32  Height;
    u16  Planes;
    u16  BitCount;
    u32  Compression;
    u32  ImageSize;
    s32  XPixPerMeter;
    s32  YPixPerMeter;
    u32  ColorUsed;
    u32  ColorImportant;
};
#pragma pack(pop)


//-------------------------------------------------------------------------------------------------
//      BMPファイルヘッダを書き込みます.
//-------------------------------------------------------------------------------------------------
void WriteBmpFileHeader( BMP_FILE_HEADER& header, FILE* pFile )
{
    fwrite( &header.Type,       sizeof(unsigned short), 1, pFile );
    fwrite( &header.Size,       sizeof(unsigned int),   1, pFile );
    fwrite( &header.Reserved1,  sizeof(unsigned short), 1, pFile );
    fwrite( &header.Reserved2,  sizeof(unsigned short), 1, pFile );
    fwrite( &header.OffBits,    sizeof(unsigned int),   1, pFile );
}


//-------------------------------------------------------------------------------------------------
//      BMP情報ヘッダを書き込みます.
//-------------------------------------------------------------------------------------------------
void WriteBmpInfoHeader( BMP_INFO_HEADER& header, FILE* pFile )
{
    fwrite( &header.Size,           sizeof(unsigned int),   1, pFile );
    fwrite( &header.Width,          sizeof(long),           1, pFile );
    fwrite( &header.Height,         sizeof(long),           1, pFile );
    fwrite( &header.Planes,         sizeof(unsigned short), 1, pFile );
    fwrite( &header.BitCount,       sizeof(unsigned short), 1, pFile );
    fwrite( &header.Compression,    sizeof(unsigned int),   1, pFile );
    fwrite( &header.ImageSize,      sizeof(unsigned int),   1, pFile );
    fwrite( &header.XPixPerMeter,   sizeof(long),           1, pFile );
    fwrite( &header.YPixPerMeter,   sizeof(long),           1, pFile );
    fwrite( &header.ColorUsed,      sizeof(unsigned int),   1, pFile );
    fwrite( &header.ColorImportant, sizeof(unsigned int),   1, pFile );
}


//-------------------------------------------------------------------------------------------------
//      BMPファイルに保存します.
//-------------------------------------------------------------------------------------------------
bool SaveToBitmap( const char16* filename, u32 width, u32 height, const u8* pBuffer )
{
    if ( filename == nullptr || pBuffer == nullptr || width == 0 || height == 0)
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    FILE* pFile;

    auto err = _wfopen_s( &pFile, filename, L"wb" );
    if ( err != 0 )
    {
        ELOG( "Error : File Open Failed." );
        return false;
    }

    // ファイルヘッダ書き込み.
    {
        BMP_FILE_HEADER header;

        header.Type      = 'MB';
        header.Size      = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER) + (width * height * 4);
        header.Reserved1 = 0;
        header.Reserved2 = 0;
        header.OffBits   = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER);

        WriteBmpFileHeader( header, pFile );
    }

    // 情報ヘッダ書き込み.
    {
        BMP_INFO_HEADER header;

        header.Size             = 40;
        header.Width            = static_cast<s32>(width);
        header.Height           = static_cast<s32>(height);
        header.Planes           = 1;
        header.BitCount         = 32;
        header.Compression      = 0;
        header.ImageSize        = width * height * 4;
        header.XPixPerMeter     = 0;
        header.YPixPerMeter     = 0;
        header.ColorUsed        = 0;
        header.ColorImportant   = 0;

        WriteBmpInfoHeader( header, pFile );
    }

    for( u32 i=0; i<height; ++i )
    {
        for( u32 j=0; j<width; ++j )
        {
            auto idx = ( i * width * 4 ) + ( j * 4 );

            fwrite( &pBuffer[idx+0], sizeof(u8), 1, pFile );
            fwrite( &pBuffer[idx+1], sizeof(u8), 1, pFile );
            fwrite( &pBuffer[idx+2], sizeof(u8), 1, pFile );
            fwrite( &pBuffer[idx+3], sizeof(u8), 1, pFile );
        }
    }

    fclose( pFile );

    // 正常終了.
    return true;
}
