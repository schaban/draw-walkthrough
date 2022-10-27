#include "crosscore.hpp"
#include "oglsys.hpp"

#include "gpu_program.hpp"

enum GPUProgFlags {
	GPUPROG_WIRE = 1
};

GPUProgram::GPUProgram() : mId(0), mFlags(0) {
	clear_inputs();
}

void GPUProgram::init(const GLuint sidVert, const GLuint sidFrag) {
	if (is_valid()) {
		nxCore::dbg_msg("GPUProgram: already initialized.\n");
		return;
	}
	GLuint pid = OGLSys::link_draw_prog(sidVert, sidFrag);
	if (pid) {
		glDetachShader(pid, sidVert);
		glDetachShader(pid, sidFrag); 
	}
	mId = pid;
	if (nxApp::get_bool_opt("wire", false)) {
		mFlags |= GPUPROG_WIRE;
	}
}

void GPUProgram::reset() {
	if (is_valid()) {
		glDeleteProgram(mId);
		mId = 0;
		mFlags = 0;
	}
}

void GPUProgram::set_gl_v4(const char* pName, const float* pVecs, const size_t nvecs) const {
	if (is_valid() && pName && pVecs && nvecs > 0) {
		GLint loc = glGetUniformLocation(mId, pName);
		if (loc >= 0) {
			glUniform4fv(loc, nvecs, pVecs);
		}
	}
}

void GPUProgram::set_gl_m(const char* pName, const float* pMtx, const size_t nmtx) const {
	if (is_valid() && pName && pMtx && nmtx > 0) {
		GLint loc = glGetUniformLocation(mId, pName);
		if (loc >= 0) {
			glUniformMatrix4fv(loc, nmtx, GL_FALSE, pMtx);
		}
	}
}

static bool ck_gl_num_elems(const int numElems) {
	bool res = false;
	switch (numElems) {
		case 1:
		case 2:
		case 3:
		case 4:
			res = true;
			break;
	}
	return res;
}

void GPUProgram::enable_gl_vertex_input(const char* pName, const int numElems, const size_t stride, const size_t offs, const bool isInt) {
	if (is_valid() && pName && ck_gl_num_elems(numElems) && stride > 0) {
		if (mInputCnt < GPUPROG_MAX_VTX_INPUTS) {
			GLint loc = glGetAttribLocation(get_id(), pName);
			if (loc >= 0) {
				glEnableVertexAttribArray(loc);
				glVertexAttribPointer(loc, numElems, isInt ? GL_INT : GL_FLOAT, GL_FALSE, (GLsizei)stride, (const void*)offs);
				mEnabledInputLocs[mInputCnt] = loc;
				++mInputCnt;
			}
		} else {
			nxCore::dbg_msg("GPUProgram: too many inputs\n");
		}
	}
}

void GPUProgram::use() {
	glUseProgram(mId);
}

void GPUProgram::set_vector(const char* pName, const cxVec& v, const float w) const {
	float tmp[] = { v.x, v.y, v.z, w };
	set_gl_v4(pName, tmp);
}

void GPUProgram::set_vector(const char* pName, const float x, const float y, const float z, const float w) const {
	float tmp[] = { x, y, z, w };
	set_gl_v4(pName, tmp);
}

void GPUProgram::set_color(const char* pName, const cxColor& c) const {
	float tmp[] = { c.r, c.g, c.b, c.a };
	set_gl_v4(pName, tmp);
}

void GPUProgram::set_matrix(const char* pName, const cxMtx& m) const {
	float tmp[4 * 4];
	m.to_mem(tmp);
	set_gl_m(pName, tmp);
}

void GPUProgram::set_transposed_matrix(const char* pName, const cxMtx& m) const {
	float tmp[4 * 4];
	m.get_transposed().to_mem(tmp);
	set_gl_m(pName, tmp);
}

void GPUProgram::set_transforms(const char* pName, const xt_xmtx* pXforms, const size_t nxforms) const {
	if (is_valid() && pName && pXforms && nxforms > 0) {
		GLint loc = glGetUniformLocation(mId, pName);
		if (loc >= 0) {
			glUniform4fv(loc, nxforms * 3, reinterpret_cast<const GLfloat*>(pXforms));
		}
	}
}

void GPUProgram::set_transform_matrices(const char* pName, const xt_xmtx* pXforms, const size_t nxforms) {
	if (is_valid() && pName && pXforms && nxforms > 0) {
		if (nxforms < XD_ARY_LEN(mMtxBuf)) {
			for (size_t i = 0; i < nxforms; ++i) {
				mMtxBuf[i] = nxMtx::mtx_from_xmtx(pXforms[i]);
			}
			set_gl_m(pName, (const float*)mMtxBuf, nxforms);
		}
	}
}

void GPUProgram::enable_vertex_float(const char* pName, const size_t stride, const size_t offs) {
	enable_gl_vertex_input(pName, 1, stride, offs);
}

void GPUProgram::enable_vertex_vec2(const char* pName, const size_t stride, const size_t offs) {
	enable_gl_vertex_input(pName, 2, stride, offs);
}

void GPUProgram::enable_vertex_vec3(const char* pName, const size_t stride, const size_t offs) {
	enable_gl_vertex_input(pName, 3, stride, offs);
}

void GPUProgram::enable_vertex_vec4(const char* pName, const size_t stride, const size_t offs) {
	enable_gl_vertex_input(pName, 4, stride, offs);
}

void GPUProgram::enable_vertex_int4(const char* pName, const size_t stride, const size_t offs) {
	enable_gl_vertex_input(pName, 4, stride, offs, true);
}

void GPUProgram::clear_inputs() {
	mInputCnt = 0;
	for (uint32_t i = 0; i < GPUPROG_MAX_VTX_INPUTS; ++i) {
		mEnabledInputLocs[i] = 0;
	}
}

void GPUProgram::disable_vertex_inputs() {
	for (uint32_t i = 0; i < mInputCnt; ++i) {
		glDisableVertexAttribArray(mEnabledInputLocs[i]);
	}
	clear_inputs();
}

void GPUProgram::draw_triangles(const size_t numTris, const size_t idxOrg, const bool idx32) {
	if (is_valid()) {
		use();
		GLsizei cnt = (GLsizei)(numTris * 3);
		GLenum typ = idx32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
		size_t siz = idx32 ? sizeof(uint32_t) : sizeof(uint16_t);
		const void* org = reinterpret_cast<const void*>(idxOrg * siz);
		if (mFlags & GPUPROG_WIRE) {
			for (size_t i = 0; i < numTris; ++i) {
				glDrawElements(GL_LINE_LOOP, 3, typ, XD_INCR_PTR(org, siz * 3 * i));
			}
		} else {
			glDrawElements(GL_TRIANGLES, cnt, typ, org);
		}
	}
}


namespace GPUShader {

GLuint load(cxResourceManager* pRsrcMgr, const char* pName, const char* pShadersPath) {
	GLuint sid = 0;
	const char* pDataPath = pRsrcMgr ? pRsrcMgr->get_data_path() : ".";
	if (pName) {
		char path[256];
		char* pPath = path;
		size_t pathLen = nxCore::str_len(pDataPath) + nxCore::str_len(pName) + 1;
		if (pShadersPath) {
			pathLen += nxCore::str_len(pShadersPath) + 2;
		} else {
			pathLen += 2;
		}
		if (pathLen > sizeof(path)) {
			pPath = reinterpret_cast<char*>(nxCore::mem_alloc(pathLen));
		}
		if (pPath) {
			XD_SPRINTF(XD_SPRINTF_BUF(pPath, pathLen), "%s/%s/%s", pDataPath, pShadersPath ? pShadersPath : "", pName);
			size_t srcSize = 0;
			char* pSrc = (char*)nxCore::bin_load(pPath, &srcSize, false, true);
			if (pSrc) {
				char* pCompileSrc = pSrc;
				size_t compileSize = srcSize;
				const char* pPrologue = nullptr;
				if (OGLSys::is_es()) {
					pPrologue = "precision highp float;\n";
				} else {
					pPrologue = "#version 150\n";
				}
				if (pPrologue) {
					size_t plLen = nxCore::str_len(pPrologue);
					compileSize += plLen;
					pCompileSrc = (char*)nxCore::mem_alloc(compileSize, "GPUShader:tmp");
					if (pCompileSrc) {
						nxCore::mem_copy(pCompileSrc, pPrologue, plLen);
						nxCore::mem_copy(pCompileSrc + plLen, pSrc, srcSize);
					}
				}
				if (pCompileSrc) {
					GLenum kind = nxCore::str_ends_with(pName, ".vert") ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
					sid = OGLSys::compile_shader_str(pCompileSrc, compileSize, kind);
					if (pCompileSrc != pSrc) {
						nxCore::mem_free(pCompileSrc);
					}
				}
				nxCore::bin_unload(pSrc);
			}
			if (pPath != path) {
				nxCore::mem_free(pPath);
			}
		}
	}
	return sid;
}

} // GPUShader

