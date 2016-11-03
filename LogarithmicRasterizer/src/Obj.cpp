//-------------------------------------------------------------------------------------------------
// File : Obj.cpp
// Desc : Wavefront OBJ Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <Obj.h>
#include <asdxLogger.h>
#include <fstream>


//-------------------------------------------------------------------------------------------------
//      MTLファイルを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadFromMTL( const char* filename, ResOBJ* pResult )
{
    if ( filename == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    std::ifstream file;
    char buf[2048];

    file.open( filename, std::ios::in );
    if ( !file.is_open() )
    {
        ELOG( "Error : File Open Failed. filename = %s", filename );
        return false;
    }

    ResMTL* material = nullptr;

    for( ;; )
    {
        file >> buf;

        if ( !file || file.eof() )
        { break; }

        if ( 0 == strcmp( buf, "newmtl") )
        {
            char name[256] = {};
            file >> name;

            ResMTL instance = {};
            instance.Name = name;

            auto index = pResult->Materials.size();
            pResult->Materials.push_back( instance );
            material = &pResult->Materials[index];
        }

        if ( material != nullptr )
        {
            if ( 0 == strcmp( buf, "Ka") )
            { file >> material->Ambient.x >> material->Ambient.y >> material->Ambient.z; }
            else if ( 0 == strcmp( buf, "Kd") )
            { file >> material->Diffuse.x >> material->Diffuse.y >> material->Diffuse.z; }
            else if ( 0 == strcmp( buf, "Ks") )
            { file >> material->Specular.x >> material->Specular.y >> material->Specular.z; }
            else if ( 0 == strcmp( buf, "d") || 0 == strcmp( buf, "Tr") )
            { file >> material->Alpha; }
            else if ( 0 == strcmp( buf, "map_Ka") )
            { file >> material->AmbientMap; }
            else if ( 0 == strcmp( buf, "map_Kd") )
            { file >> material->DiffuseMap; }
            else if ( 0 == strcmp( buf, "map_Ks") )
            { file >> material->SpecularMap; }
            else if ( 0 == strcmp( buf, "map_Bump") )
            { file >> material->BumpMap; }
        }

        file.ignore( 2048, '\n' );
    }

    pResult->Materials.shrink_to_fit();

    file.close();

    return true;
}

//-------------------------------------------------------------------------------------------------
//      OBJファイルを読み込みます.
//-------------------------------------------------------------------------------------------------
bool LoadFromOBJ( const char* filename, ResOBJ* pResult )
{
    if ( filename == nullptr || pResult == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    std::ifstream file;
    file.open( filename, std::ios::in );

    if ( !file.is_open() )
    {
        ELOG( "Error : File Open Failed. filename = %s", filename );
        return false;
    }

    char buf[2048];

    std::vector<asdx::Vector3> positions;
    std::vector<asdx::Vector3> normals;
    std::vector<asdx::Vector2> texcoords;
    u32 offset = 0;

    for( ;; )
    {
        file >> buf;

        if ( !file || file.eof() )
        { break; }

        if ( 0 == strcmp( buf, "#" ) )
        { continue; }
        else if ( 0 == strcmp( buf, "v" ) )
        {
            asdx::Vector3 val;
            file >> val.x >> val.y >> val.z;
            positions.push_back( val );
        }
        else if ( 0 == strcmp( buf, "vt" ) )
        {
            asdx::Vector2 val;
            file >> val.x >> val.y;
            texcoords.push_back( val );
        }
        else if ( 0 == strcmp( buf, "vn" ) )
        {
            asdx::Vector3 val;
            file >> val.x >> val.y >> val.z;
            normals.push_back( val );
        }
        else if ( 0 == strcmp( buf, "f" ) )
        {
            offset++;
            u32 indexP[4] = {U32_MAX, U32_MAX, U32_MAX, U32_MAX};
            u32 indexU[4] = {U32_MAX, U32_MAX, U32_MAX, U32_MAX};
            u32 indexN[4] = {U32_MAX, U32_MAX, U32_MAX, U32_MAX};

            auto i = 0;
            for( i=0; i<4; ++i )
            {
                u32 index;
                file >> index;
                indexP[i] = index - 1;

                if ( '/' == file.peek() )
                {
                    file.ignore();

                    if ( '/' != file.peek() )
                    {
                        file >> index;
                        indexU[i] = index - 1;
                    }

                    if ( '/' == file.peek() )
                    {
                        file.ignore();
                        file >> index;
                        indexN[i] = index - 1;
                    }
                }

                if ( i < 3 )
                {
                    auto idx = u32(pResult->Positions.size());
                    pResult->Indices.push_back(idx);

                    if ( indexP[i] != U32_MAX )
                    { pResult->Positions.push_back( positions[indexP[i]] ); }
                    if ( indexU[i] != U32_MAX )
                    { pResult->TexCoords.push_back( texcoords[indexU[i]] ); }
                    if ( indexN[i] != U32_MAX )
                    { pResult->Normals.push_back( normals[indexN[i]] ); }
                }

                if ( '\n' == file.peek() || '\r' == file.peek() )
                { break; }
            }

            if ( i >= 3 )
            {
                offset++;

                for( auto j=1; j<4; ++j )
                {
                    auto k = (j + 1) % 4;

                    auto idx = u32(pResult->Positions.size());
                    pResult->Indices.push_back(idx);

                    if ( indexP[k] != U32_MAX )
                    { pResult->Positions.push_back( positions[indexP[k]] ); }
                    if ( indexU[k] != U32_MAX )
                    { pResult->TexCoords.push_back( texcoords[indexU[k]] ); }
                    if ( indexN[k] != U32_MAX )
                    { pResult->Normals.push_back( normals[indexN[k]] ); }
                }
            }
        }
        else if ( 0 == strcmp( buf, "mtllib") )
        {
            char path[256] = {};
            file >> path;

            //LoadFromMTL( path, pResult );
        }
        else if ( 0 == strcmp( buf, "usemtl") )
        {
            char name[256] = {};
            file >> name;

            ResSubset instance = {};
            instance.Name   = name;
            instance.Offset = offset * 3;
            instance.Count  = 0;

            pResult->Subsets.push_back( instance );
        }

        file.ignore( 2048, '\n' );
    }

    file.close();

    {
        auto index = pResult->Subsets.size();
        if ( index > 0 )
        {
            pResult->Subsets[index - 1].Count = offset * 3 - pResult->Subsets[index - 1].Offset;

            for( s32 i=s32(index)-2; i>=0; i-- )
            {
                pResult->Subsets[i].Count = pResult->Subsets[i+1].Offset - pResult->Subsets[i].Offset;
            }
        }
    }

    positions.clear();
    texcoords.clear();
    normals  .clear();

    pResult->Positions  .shrink_to_fit();
    pResult->Normals    .shrink_to_fit();
    pResult->TexCoords  .shrink_to_fit();
    pResult->Subsets    .shrink_to_fit();
    pResult->Indices    .shrink_to_fit();

    return true;
}
