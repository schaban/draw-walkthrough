struct GPUGeom {
	xt_float3   pos;
	xt_float3   nrm;
	xt_texcoord tex;
	xt_rgba     clr;
};

struct GPUSkin {
	xt_float4 wgt;
	xt_int4   idx;
};

namespace GPUModel {
	bool is_prepared(const sxModelData* pMdl);
	void prepare(sxModelData* pMdl);
	void release(sxModelData* pMdl);
}
