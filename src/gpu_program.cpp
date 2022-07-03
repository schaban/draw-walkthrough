#include "crosscore.hpp"
#include "oglsys.hpp"

#include "gpu_program.hpp"

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
}

void GPUProgram::reset() {
	if (is_valid()) {
		glDeleteProgram(mId);
		mId = 0;
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

void GPUProgram::enable_gl_vertex_input(const char* pName, const int numElems, const size_t stride, const size_t offs) {
	if (is_valid() && pName && ck_gl_num_elems(numElems) && stride > 0) {
		GLint loc = glGetAttribLocation(get_id(), pName);
		if (loc >= 0) {
			glEnableVertexAttribArray(loc);
			glVertexAttribPointer(loc, numElems, GL_FLOAT, GL_FALSE, (GLsizei)stride, (const void*)offs);
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


namespace GPUProgUtils {

GLuint load_shader(cxResourceManager* pRsrcMgr, const char* pName, const char* pShadersPath) {
	GLuint sid = 0;
	const char* pDataPath = pRsrcMgr ? pRsrcMgr->get_data_path() : ".";
	if (pName) {
		char path[256];
		char* pPath = path;
		char pathLen = nxCore::str_len(pDataPath) + nxCore::str_len(pName) + 1;
		if (pShadersPath) {
			pathLen += nxCore::str_len(pShadersPath) + 2;
		} else {
			pathLen += 1;
		}
		if (pathLen > sizeof(path)) {
			pPath = (char*)nxCore::mem_alloc(pathLen);
		}
		if (pPath) {
			XD_SPRINTF(XD_SPRINTF_BUF(pPath, pathLen), "%s/%s/%s", pDataPath, pShadersPath ? pShadersPath : "", pName);
			size_t srcSize = 0;
			char* pSrc = (char*)nxCore::bin_load(path, &srcSize, false, true);
			if (pSrc) {
				GLenum kind = nxCore::str_ends_with(pName, ".vert") ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
				sid = OGLSys::compile_shader_str(pSrc, srcSize, kind);
				nxCore::bin_unload(pSrc);
			}
			if (pPath != path) {
				nxCore::mem_free(pPath);
			}
		}
	}
	return sid;
}

} // GPUProgUtils

