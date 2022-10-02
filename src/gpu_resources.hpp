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
	GLuint get_vertex_buffer(sxModelData* pMdl);
	GLuint get_index_buffer(sxModelData* pMdl);
}

namespace GPUTexture {
	void prepare(sxTextureData* pTex);
	void release(sxTextureData* pTex);
}

namespace GPUResources {
	void init_manager_ifc(cxResourceManager* pMgr);
}
