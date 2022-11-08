#include "crosscore.hpp"
#include "oglsys.hpp"

#include "gpu_resources.hpp"

namespace GPUModel {

enum {
	GPUWK_VTX_BUF = 0,
	GPUWK_IDX_BUF = 1
};

bool is_prepared(const sxModelData* pMdl) {
	bool res = false;
	if (pMdl) {
		const GLuint* pBufMem = pMdl->get_gpu_wk<GLuint>();
		const GLuint* pVB = &pBufMem[GPUWK_VTX_BUF];
		const GLuint* pIB = &pBufMem[GPUWK_IDX_BUF];
		res = *pVB && *pIB;
	}
	return res;
}

static void create_vertex_buffer(sxModelData* pMdl) {
	int numPoints = pMdl->mPntNum;
	if (numPoints < 1) return;
	bool skinFlg = pMdl->has_skin();
	size_t vtxSize = sizeof(GPUGeom);
	if (skinFlg) {
		vtxSize += sizeof(GPUSkin);
	}
	GLuint* pVB = &pMdl->get_gpu_wk<GLuint>()[GPUWK_VTX_BUF];
	if (*pVB) return;
	glGenBuffers(1, pVB);
	if (*pVB) {
		size_t vtxMemSize = numPoints * vtxSize;
		void* pVtxMem = nxCore::mem_alloc(vtxMemSize, "TmpVtxMem");
		if (pVtxMem) {
			GPUGeom* pVtxGeom = reinterpret_cast<GPUGeom*>(pVtxMem);
			GPUSkin* pVtxSkin = skinFlg ? reinterpret_cast<GPUSkin*>(pVtxGeom + 1) : nullptr;
			for (int i = 0; i < numPoints; ++i) {
				pVtxGeom->pos = pMdl->get_pnt_pos(i);
				pVtxGeom->nrm = pMdl->get_pnt_nrm(i);
				pVtxGeom->tex = pMdl->get_pnt_tex(i);
				pVtxGeom->clr = pMdl->get_pnt_clr(i);
				if (pVtxSkin) {
					sxModelData::PntSkin pntSkin = pMdl->get_pnt_skin(i);
					pVtxSkin->wgt.fill(0.0f);
					pVtxSkin->idx.fill(0.0f);
					for (int j = 0; j < pntSkin.num; ++j) {
						pVtxSkin->wgt.set_at(j, pntSkin.wgt[j]);
						pVtxSkin->idx.set_at(j, float(pntSkin.idx[j]));
					}
					pVtxSkin = reinterpret_cast<GPUSkin*>(XD_INCR_PTR(pVtxSkin, vtxSize));
				}
				pVtxGeom = reinterpret_cast<GPUGeom*>(XD_INCR_PTR(pVtxGeom, vtxSize));
			}
			glBindBuffer(GL_ARRAY_BUFFER, *pVB);
			glBufferData(GL_ARRAY_BUFFER, vtxMemSize, pVtxMem, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			nxCore::mem_free(pVtxMem);
		}
	}
}

static void create_index_buffer(sxModelData* pMdl) {
	GLuint* pIB = &pMdl->get_gpu_wk<GLuint>()[GPUWK_IDX_BUF];
	if (*pIB) return;
	int numIndices = pMdl->mIdx16Num;
	if (numIndices < 3) return;
	const uint16_t* pIdxData = pMdl->get_idx16_top();
	if (pIdxData) {
		glGenBuffers(1, pIB);
		if (*pIB) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *pIB);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint16_t), pIdxData, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}
}

void prepare(sxModelData* pMdl) {
	if (!pMdl) return;
	if (is_prepared(pMdl)) return;
	create_vertex_buffer(pMdl);
	create_index_buffer(pMdl);
}

void release(sxModelData* pMdl) {
	if (!pMdl) return;
	GLuint* pBufMem = pMdl->get_gpu_wk<GLuint>();
	GLuint* pVB = &pBufMem[GPUWK_VTX_BUF];
	if (*pVB) {
		glDeleteBuffers(1, pVB);
		*pVB = 0;
	}
	GLuint* pIB = &pBufMem[GPUWK_IDX_BUF];
	if (*pIB) {
		glDeleteBuffers(1, pIB);
		*pIB = 0;
	}
	pMdl->clear_tex_wk();
}

GLuint get_vertex_buffer(sxModelData* pMdl) {
	GLuint vb = 0;
	if (pMdl) {
		prepare(pMdl);
		const GLuint* pBufMem = pMdl->get_gpu_wk<GLuint>();
		const GLuint* pVB = &pBufMem[GPUWK_VTX_BUF];
		vb = *pVB;
	}
	return vb;
}

GLuint get_index_buffer(sxModelData* pMdl) {
	GLuint ib = 0;
	if (pMdl) {
		prepare(pMdl);
		const GLuint* pBufMem = pMdl->get_gpu_wk<GLuint>();
		const GLuint* pIB = &pBufMem[GPUWK_IDX_BUF];
		ib = *pIB;
	}
	return ib;
}

} /* GPUModel */


namespace GPUTexture {

void prepare(sxTextureData* pTex) {
}

void release(sxTextureData* pTex) {
}

} /* GPUTexture */


namespace GPUResources {

void init_manager_ifc(cxResourceManager* pMgr) {
	if (!pMgr) return;
	cxResourceManager::GfxIfc rsrcGfxIfc;
	rsrcGfxIfc.reset();
	rsrcGfxIfc.prepareTexture = GPUTexture::prepare;
	rsrcGfxIfc.releaseTexture = GPUTexture::release;
	rsrcGfxIfc.prepareModel = GPUModel::prepare;
	rsrcGfxIfc.releaseModel = GPUModel::release;
	pMgr->set_gfx_ifc(rsrcGfxIfc);
}

} /* GPUResources */

