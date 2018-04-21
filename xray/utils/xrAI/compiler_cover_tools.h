#pragma once

typedef xr_vector<bool> COVER_NODES;
extern COVER_NODES g_cover_nodes;

// volumetric query
DEF_VECTOR(Nearest, u32);

class Query
{
public:
    Nearest q_List;
    Nearest q_Clear;
    xr_hash_map<u32, bool> q_Marks;
    Fvector q_Base;

    IC void Begin(int count)
    {
        q_List.reserve(8192);
        q_Clear.reserve(8192);
    }

    IC void Init(const Fvector& P)
    {
        q_Base.set(P);
        q_List.clear();
        q_Clear.clear();
    }

    IC void Perform(u32 ID)
    {
        if (ID == InvalidNode)
            return;
        if (q_Marks[ID])
            return;

        q_Marks[ID] = true;
        q_Clear.push_back(ID);

        vertex& N = g_nodes[ID];
        if (q_Base.distance_to_sqr(N.Pos) > cover_sqr_dist)
            return;

        // ok
        q_List.push_back(ID);

        Perform(N.n1);
        Perform(N.n2);
        Perform(N.n3);
        Perform(N.n4);
    }

    IC void Clear()
    {
        q_Marks.clear();
    }
};

// -------------------------------- Ray pick
typedef Fvector RayCache[3];
struct RC {
    RayCache C;
};

IC float getLastRP_Scale(CDB::COLLIDER* DB, RayCache& C)
{
    u32 tris_count = DB->r_count();
    float scale = 1.f;
    Fvector B;

    for (u32 I = 0; I < tris_count; I++) {
        CDB::RESULT& rpinf = DB->r_begin()[I];
        // Access to texture
        //			CDB::TRI& clT								=
        Level.get_tris()[rpinf.id];
        b_rc_face& F = g_rc_faces[rpinf.id];

        if (F.dwMaterial >= g_materials.size())
            Msg("[%d] -> [%d]", F.dwMaterial, g_materials.size());
        b_material& M = g_materials[F.dwMaterial];
        b_texture& T = g_textures[M.surfidx];
        Shader_xrLCVec& LIB = g_shaders_xrlc->Library();
        if (M.shader_xrlc >= LIB.size())
            return 0; //. hack
        Shader_xrLC& SH = LIB[M.shader_xrlc];
        if (!SH.flags.bLIGHT_CastShadow)
            continue;

        if (0 == T.pSurface)
            T.bHasAlpha = FALSE;
        if (!T.bHasAlpha) {
            // Opaque poly - cache it
            C[0].set(rpinf.verts[0]);
            C[1].set(rpinf.verts[1]);
            C[2].set(rpinf.verts[2]);
            return 0;
        }

        // barycentric coords
        // note: W,U,V order
        B.set(1.0f - rpinf.u - rpinf.v, rpinf.u, rpinf.v);

        // calc UV
        Fvector2* cuv = F.t;
        Fvector2 uv;
        uv.x = cuv[0].x * B.x + cuv[1].x * B.y + cuv[2].x * B.z;
        uv.y = cuv[0].y * B.x + cuv[1].y * B.y + cuv[2].y * B.z;

        int U = iFloor(uv.x * float(T.dwWidth) + .5f);
        int V = iFloor(uv.y * float(T.dwHeight) + .5f);
        U %= T.dwWidth;
        if (U < 0)
            U += T.dwWidth;
        V %= T.dwHeight;
        if (V < 0)
            V += T.dwHeight;

        u32 pixel = T.pSurface[V * T.dwWidth + U];
        u32 pixel_a = color_get_A(pixel);
        float opac = 1.f - float(pixel_a) / 255.f;
        scale *= opac;
    }

    return scale;
}

IC float rayTrace(CDB::COLLIDER* DB, Fvector& P, Fvector& D, float R, RayCache& C)
{
    R_ASSERT(DB);

    // 1. Check cached polygon
    float _u, _v, range;
    bool res = CDB::TestRayTri(P, D, C, _u, _v, range, false);
    if (res) {
        if (range > 0 && range < R)
            return 0;
    }

    // 2. Polygon doesn't pick - real database query
    DB->ray_query(&Level, P, D, R);

    // 3. Analyze polygons and cache nearest if possible
    if (0 == DB->r_count()) {
        return 1;
    } else {
        return getLastRP_Scale(DB, C);
    }
    //	return 0;
}

IC int calcSphereSector(Fvector& dir)
{
    Fvector2 flat;

    // flatten
    flat.set(dir.x, dir.z);
    flat.norm();

    // analyze
    if (_abs(flat.x) > _abs(flat.y)) {
        // sector 0,7,3,4
        if (flat.x < 0) {
            // sector 3,4
            if (flat.y > 0)
                return 3;
            else
                return 4;
        } else {
            // sector 0,7
            if (flat.y > 0)
                return 0;
            else
                return 7;
        }
    } else {
        // sector 1,2,6,5
        if (flat.x < 0) {
            // sector 2,5
            if (flat.y > 0)
                return 2;
            else
                return 5;
        } else {
            // sector 1,6
            if (flat.y > 0)
                return 1;
            else
                return 6;
        }
    }
}