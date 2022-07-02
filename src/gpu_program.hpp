class GPUProgram {
protected:
	GLuint mId;

	void set_gl_v4(const char* pName, const float* pVecs, const size_t nvecs = 1) const;
	void set_gl_m(const char* pName, const float* pMtx, const size_t nmtx = 1) const;
	void enable_gl_vertex_input(const char* pName, const int numElems, const size_t stride, const size_t offs);

public:
	GPUProgram() : mId(0) {}

	void init(const GLuint sidVert, const GLuint sidFrag);
	void reset();
	GLuint get_id() const { return mId; }
	bool is_valid() const { return mId != 0; }

	void use();
	void set_vector(const char* pName, const cxVec& v, const float w = 0.0f) const;
	void set_vector(const char* pName, const float x, const float y, const float z, const float w) const;
	void set_color(const char* pName, const cxColor& c) const;
	void set_matrix(const char* pName, const cxMtx& m) const;
	void set_transposed_matrix(const char* pName, const cxMtx& m) const;
	void set_transforms(const char* pName, const xt_xmtx* pXforms, const size_t nxforms) const;
	void enable_vertex_float(const char* pName, const size_t stride, const size_t offs);
	void enable_vertex_vec2(const char* pName, const size_t stride, const size_t offs);
	void enable_vertex_vec3(const char* pName, const size_t stride, const size_t offs);
	void enable_vertex_vec4(const char* pName, const size_t stride, const size_t offs);
};

namespace GPUProgUtils {
	GLuint load_shader(cxResourceManager* pRsrcMgr, const char* pName, const char* pShadersPath);
}
