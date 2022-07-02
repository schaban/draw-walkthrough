class GPUProgram {
protected:
	GLuint mId;

	void set_gl_v4(const char* pName, const float* pVecs, const size_t nvecs = 1) const;
	void set_gl_m(const char* pName, const float* pMtx, const size_t nmtx = 1) const;

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
};

namespace GPUProgUtils {
	GLuint load_shader(cxResourceManager* pRsrcMgr, const char* pName, const char* pShadersPath);
}
