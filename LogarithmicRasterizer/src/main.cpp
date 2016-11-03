//-------------------------------------------------------------------------------------------------
// File : main.cpp
// Desc : Application Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxMath.h>
#include <asdxLogger.h>
#include <vector>
#include <Bmp.h>
#include <Obj.h>


//-------------------------------------------------------------------------------------------------
// Using Statements
//-------------------------------------------------------------------------------------------------
using namespace asdx;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    Vector3     Position;       //!< 位置座標です.
};


//-------------------------------------------------------------------------------------------------
//      2次元ベクトルに変換します.
//-------------------------------------------------------------------------------------------------
Vector2 ToVector2( const Vector4& value )
{ return Vector2( value.x, value.y ); }


//-------------------------------------------------------------------------------------------------
//      3次元ベクトルに変換します.
//-------------------------------------------------------------------------------------------------
Vector3 ToVector3( const Vector4& value )
{ return Vector3( value.x, value.y, value.z ); }


//-------------------------------------------------------------------------------------------------
//      正規化デバイス座標系に変換します。[-1, 1]の範囲です.
//-------------------------------------------------------------------------------------------------
Vector4 ToNDC( const Vector4& value )
{
    return Vector4( 
        value.x / value.w,
        value.y / value.w,
        value.z / value.w,
        1.0f / value.w );
}

//-------------------------------------------------------------------------------------------------
//      スクリーン空間座標系に変換します. [0, 1]の範囲です.
//-------------------------------------------------------------------------------------------------
Vector4 ToSS( const Vector4& value )
{
    return Vector4( 
        value.x * 0.5f + 0.5f,
        value.y * 0.5f + 0.5f,
        value.z,
        value.w ); 
}

//-------------------------------------------------------------------------------------------------
//      デバイス座標系に変換します. (ビューポートサイズの範囲内).
//-------------------------------------------------------------------------------------------------
Vector4 ToDC( const Vector4& value, f32 w, f32 h )
{
    return Vector4(
        value.x * w,
        value.y * h,
        value.z,
        value.w );
}

//-------------------------------------------------------------------------------------------------
//      対数変換を行います.
//-------------------------------------------------------------------------------------------------
Vector4 LogTransform( const Vector4& value, f32 n, f32 f )
{
    // Logarithmic Perspective Shadow Map, 
    // Chapter 7 Logarithmic rasterization hardware, p.149, Equation 7.1

    auto c0 = -1.0f / log(f / n);
    auto c1 = (1.0f - (f / n)) / (f / n);

    return Vector4(
        value.x,
        c0 * log(c1 * value.y + 1.0f),
        value.z,
        value.w);
}

//-------------------------------------------------------------------------------------------------
//      逆対数変換を行います.
//-------------------------------------------------------------------------------------------------
Vector2 InvLogTransform( const Vector2& value, f32 n, f32 f, f32 h )
{
    // Logarithmic Perspective Shadow Map, 
    // Chapter 7 Logarithmic rasterization hardware, p.152, Equation 7.4

    auto y = value.y / h;       // [0, 1] の範囲に戻します.
    auto c0 = -1.0f / log(f / n);
    auto c1 = (1.0f - (f / n)) / (f / n);

    return Vector2(
        value.x,
        ((exp(y / c0) - 1.0f) / c1) * h);   // 逆変換してから, [0, h] の範囲に戻します.
}

//-------------------------------------------------------------------------------------------------
//      2次元ベクトルの外積を求めます.
//-------------------------------------------------------------------------------------------------
f32 CrossProduct( const Vector2& a, const Vector2& b )
{ return a.x * b.y - b.x * a.y; }


//-------------------------------------------------------------------------------------------------
//      メインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    const char16* filename = L"color.bmp";

    Vector3 position = Vector3(0.0f, 0.0f, 350.0f);
    Vector3 target   = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 upward   = Vector3(0.0f, 1.0f, 0.0f);

    f32 fov      = F_PIDIV4;
    f32 nearClip = 1.0f;
    f32 farClip  = 1000.0f;

    struct Vertex
    {
        Vector3 Position;
        Vector2 TexCoord;
        Vector4 Color;

        Vertex()
        {}

        Vertex(const Vector3& p, const Vector2& t, const Vector4& c)
            : Position(p)
            , TexCoord(t)
            , Color(c)
        { /* DO_NOTHING */ }
    };

    // 入力頂点座標.
    std::vector<Vertex> vertices;
    {
        vertices.resize(9);
        vertices[0] = Vertex( Vector3(-100.0f, -100.0f, 100.0f), Vector2(0.0f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) );
        vertices[1] = Vertex( Vector3( 100.0f, -100.0f, 100.0f), Vector2(1.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f) );
        vertices[2] = Vertex( Vector3(   0.0f,  50.0f,  100.0f), Vector2(0.0f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f) );

        vertices[3] = Vertex( Vector3(-150.0f, -80.0f, 50.0f), Vector2(0.0f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f) );
        vertices[4] = Vertex( Vector3(  50.0f, -80.0f, 50.0f), Vector2(1.0f, 0.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f) );
        vertices[5] = Vertex( Vector3( -50.0f,  70.0f, 50.0f), Vector2(0.0f, 1.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f) );

        vertices[6] = Vertex( Vector3(-200.0f, -50.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) );
        vertices[7] = Vertex( Vector3( 200.0f, -50.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) );
        vertices[8] = Vertex( Vector3(   0.0f,  240.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) );
    }

    // 画像サイズ.
    u32 width  = 960;
    u32 height = 540;

    auto w = f32(width);
    auto h = f32(height);

    auto stepX = 1.0f / w;
    auto stepY = 1.0f / h;

    // 変換行列.
    auto World = Matrix::CreateIdentity();
    auto View  = Matrix::CreateLookAt( position, target, upward );
    auto Proj  = Matrix::CreatePerspectiveFieldOfView( fov, w / h, nearClip, farClip );
    auto ViewProj = View * Proj;

    // レンダーターゲット.
    auto colorBuffer = new u8 [ width * height * 4 ];
    auto depthBuffer = new f32 [ width * height ];

    // レンダーターゲットをクリア.
    for( u32 i=0; i<height; ++i )
    {
        for( u32 j=0; j<width; ++j )
        {
            auto idx = i * width * 4 + j * 4;
            colorBuffer[idx + 0] = 255;
            colorBuffer[idx + 1] = 255;
            colorBuffer[idx + 2] = 255;
            colorBuffer[idx + 3] = 255;

            idx = i * width + j;
            depthBuffer[idx] = F32_MAX;
        }
    }

    auto c0 = -1.0f / log(farClip / nearClip);
    auto c1 = (1.0f - (farClip / nearClip)) / (farClip / nearClip);

    for( size_t Index=0; Index < vertices.size(); Index += 3 )
    {
        auto& v0 = vertices[Index + 0];
        auto& v1 = vertices[Index + 1];
        auto& v2 = vertices[Index + 2];

        // ワールド変換.
        auto P0w = Vector4::Transform( Vector4( v0.Position, 1.0f ), World );
        auto P1w = Vector4::Transform( Vector4( v1.Position, 1.0f ), World );
        auto P2w = Vector4::Transform( Vector4( v2.Position, 1.0f ), World );

        // ビュー射影変換.
        auto P0p = Vector4::Transform( P0w, ViewProj );
        auto P1p = Vector4::Transform( P1w, ViewProj );
        auto P2p = Vector4::Transform( P2w, ViewProj );

        // 正規化デバイス座標系に変換.
        P0p = ToNDC( P0p );
        P1p = ToNDC( P1p );
        P2p = ToNDC( P2p );

        // スクリーン空間座標系に変換.
        auto P0s = ToSS( P0p );
        auto P1s = ToSS( P1p );
        auto P2s = ToSS( P2p );

        P0p = LogTransform( P0s, nearClip, farClip );
        P1p = LogTransform( P1s, nearClip, farClip );
        P2p = LogTransform( P2s, nearClip, farClip );

        P0p = ToDC( P0p, w, h );
        P1p = ToDC( P1p, w, h );
        P2p = ToDC( P2p, w, h );

        P0s = ToDC( P0s, w, h );
        P1s = ToDC( P1s, w, h );
        P2s = ToDC( P2s, w, h );

        auto mini = Vector2::Min(ToVector2(P0p), Vector2::Min(ToVector2(P1p), ToVector2(P2p)));
        auto maxi = Vector2::Max(ToVector2(P0p), Vector2::Max(ToVector2(P1p), ToVector2(P2p)));

        // ビューポートでクリッピング.
        mini = Vector2::Max( mini, Vector2(0.0f, 0.0f) );
        maxi = Vector2::Min( maxi, Vector2(w, h) );

        if ( mini.x > maxi.x && mini.y > maxi.y )
        { continue; }

        // ラスタライズ処理.
        {
            auto vs1 = ToVector2(P1s) - ToVector2(P0s);
            auto vs2 = ToVector2(P2s) - ToVector2(P0s);

            float div = CrossProduct( vs1, vs2 );

            // ピクセルサイズに合わせる.
            auto TriMin = Vector2(floor(mini.x), floor(mini.y));
            auto TriMax = Vector2(ceil(maxi.x), ceil(maxi.y));

            // ピクセル中心まで移動.
            TriMin += Vector2(0.5f, 0.5f);
            TriMax += Vector2(0.5f, 0.5f);

            Vector2 vPos;
            for( vPos.y = TriMin.y; vPos.y < TriMax.y; vPos.y++ )
            {
                for( vPos.x = TriMin.x; vPos.x < TriMax.x; vPos.x++ )
                {
                    // 逆対数変換をかけて線形な値を取ってくる.
                    auto p = InvLogTransform( vPos - ToVector2( P0p ), nearClip, farClip, h );

                    auto s = CrossProduct( p, vs2 ) / div;
                    auto t = CrossProduct( vs1, p ) / div;

                    if ( s >= 0.0f && t >= 0.0f && (s+t) <= 1.0f )
                    {
                        auto col = v0.Color    * s + v1.Color    * t + v2.Color    * ( 1.0f - s - t ); 
                        auto tex = v0.TexCoord * s + v1.TexCoord * t + v2.TexCoord * ( 1.0f - s - t );
                       
                        auto z = P0p.z * s + P1p.z * t + P2p.z * ( 1.0f - s - t );
                        auto w = P0p.w * s + P1p.w * t + P2p.w * ( 1.0f - s - t );
                        auto depth = (z / w);

                        auto idxC = s32(vPos.y) * width * 4 + s32(vPos.x) * 4;
                        auto idxD = s32(vPos.y) * width + s32(vPos.x);

                        // 深度値を比較.
                        if ( depthBuffer[idxD] >= depth )
                        {
                            colorBuffer[idxC + 0] = asdx::Clamp( int(col.x * 255.0f), 0, 255 );
                            colorBuffer[idxC + 1] = asdx::Clamp( int(col.y * 255.0f), 0, 255 );
                            colorBuffer[idxC + 2] = asdx::Clamp( int(col.z * 255.0f), 0, 255 );
                            colorBuffer[idxC + 3] = asdx::Clamp( int(col.w * 255.0f), 0, 255 );

                            depthBuffer[idxD] = depth;
                        }
                    }
                }
            }
        }
    }

    // 最終結果を出力.
    SaveToBitmap( filename, width, height, colorBuffer );

    // メモリを解放.
    SafeDeleteArray( colorBuffer );
    SafeDeleteArray( depthBuffer );

    vertices.clear();

    return 0;
}