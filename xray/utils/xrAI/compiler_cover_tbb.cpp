#include "stdafx.h"
#include "cl_intersect.h"
#include "compiler.h"
#include "compiler_cover_tools.h"
#include <tbb/tbb.h>

typedef float Cover[4];
void compute_cover_value(u32 const& N, vertex& BaseNode, float const& cover_height, Cover& cover, Query& Q,
    CDB::COLLIDER& DB, xr_hash_map<u32, RC>& cache)
{
    Fvector& BasePos = BaseNode.Pos;
    Fvector TestPos = BasePos;
    TestPos.y += cover_height;

    float c_total[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    float c_passed[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    // perform volumetric query
    Q.Perform(N, BasePos);

    // main cycle: trace rays and compute counts
    for (Nearest_it it = Q.q_List.begin(); it != Q.q_List.end(); it++) {
        // calc dir & range
        u32 ID = *it;
        R_ASSERT(ID < g_nodes.size());
        if (N == ID)
            continue;
        vertex& N = g_nodes[ID];
        Fvector& Pos = N.Pos;
        Fvector Dir;
        Dir.sub(Pos, BasePos);
        float range = Dir.magnitude();
        Dir.div(range);

        // raytrace
        int sector = calcSphereSector(Dir);
        c_total[sector] += 1.f;
        c_passed[sector] += rayTrace(&DB, TestPos, Dir, range, cache[ID].C); //
    }

    // analyze probabilities
    float value[8];
    for (int dirs = 0; dirs < 8; dirs++) {
        R_ASSERT(c_passed[dirs] <= c_total[dirs]);
        if (c_total[dirs] == 0)
            value[dirs] = 0;
        else
            value[dirs] = float(c_passed[dirs]) / float(c_total[dirs]);
        clamp(value[dirs], 0.f, 1.f);
    }

    if (value[0] < .999f) {
        value[0] = value[0];
    }

    cover[0] = (value[2] + value[3] + value[4] + value[5]) / 4.f;
    clamp(cover[0], 0.f, 1.f); // left
    cover[1] = (value[0] + value[1] + value[2] + value[3]) / 4.f;
    clamp(cover[1], 0.f, 1.f); // forward
    cover[2] = (value[6] + value[7] + value[0] + value[1]) / 4.f;
    clamp(cover[2], 0.f, 1.f); // right
    cover[3] = (value[4] + value[5] + value[6] + value[7]) / 4.f;
    clamp(cover[3], 0.f, 1.f); // back
}

void xrCover_tbb()
{
    tbb::atomic<size_t> progress{ 0 };
    tbb::parallel_for(tbb::blocked_range<size_t>(0, g_nodes.size()), [&progress](const tbb::blocked_range<size_t>& r) {
        xr_hash_map<u32, RC> cache;
        CDB::COLLIDER DB;
        DB.ray_options(CDB::OPT_CULL);
        Query Q;
        FPU::m24r();
        for (size_t N = r.begin(); N != r.end(); ++N) {
            vertex& BaseNode = g_nodes[N];
            if (!g_cover_nodes[N]) {
                BaseNode.high_cover[0] = flt_max;
                BaseNode.high_cover[1] = flt_max;
                BaseNode.high_cover[2] = flt_max;
                BaseNode.high_cover[3] = flt_max;
                BaseNode.low_cover[0] = flt_max;
                BaseNode.low_cover[1] = flt_max;
                BaseNode.low_cover[2] = flt_max;
                BaseNode.low_cover[3] = flt_max;
                continue;
            }

            compute_cover_value(N, BaseNode, high_cover_height, BaseNode.high_cover, Q, DB, cache);
            compute_cover_value(N, BaseNode, low_cover_height, BaseNode.low_cover, Q, DB, cache);
        }
        size_t prev = progress.fetch_and_add(r.size());
        Progress((prev + r.size()) * 1.0f / g_nodes.size());
    });
}