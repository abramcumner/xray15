#pragma once

template<class Collector>
void SaveAsSMF(LPCSTR fname, Collector& CL)
{
	IWriter* W = FS.w_open(fname);
	string256 tmp;
	// vertices
	for (u32 v_idx = 0; v_idx<CL.getVS(); v_idx++) {
		Fvector* v = CL.getV() + v_idx;
		sprintf(tmp, "v %f %f %f", v->x, v->y, -v->z);
		W->w_string(tmp);
	}
	// transfer faces
	for (u32 f_idx = 0; f_idx<CL.getTS(); f_idx++) {
		const auto& t = CL.getT(f_idx);
		sprintf(tmp, "f %d %d %d", t.verts[0] + 1, t.verts[2] + 1, t.verts[1] + 1);
		W->w_string(tmp);
	}
	FS.w_close(W);
}