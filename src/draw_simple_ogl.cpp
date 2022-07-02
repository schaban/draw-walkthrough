#include "crosscore.hpp"
#include "oglsys.hpp"
#include "draw.hpp"

#include "gpu_program.hpp"

DRW_IMPL_BEGIN

static cxResourceManager* s_pRsrcMgr = nullptr;

static Draw::Font* s_pFont = nullptr;
static GLuint s_fontVBO = 0;
static GLuint s_fontIBO = 0;

static GPUProgram s_progSymbol;
static GPUProgram s_progStatic;
static GPUProgram s_progDeform;


static GLuint load_shader(const char* pName) {
	return GPUProgUtils::load_shader(s_pRsrcMgr, pName, "simple_ogl");
}


static void prepare_model(sxModelData* pMdl) {
	// ...
}

static void release_model(sxModelData* pMdl) {
	// ...
}

static void prepare_texture(sxTextureData* pTex) {
	// ...
}

static void release_texture(sxTextureData* pTex) {
	// ...
}

static void init_rsrc_mgr() {
	cxResourceManager::GfxIfc rsrcGfxIfc;
	rsrcGfxIfc.reset();
	rsrcGfxIfc.prepareTexture = prepare_texture;
	rsrcGfxIfc.releaseTexture = release_texture;
	rsrcGfxIfc.prepareModel = prepare_model;
	rsrcGfxIfc.releaseModel = release_model;
	s_pRsrcMgr->set_gfx_ifc(rsrcGfxIfc);
}

static void init_gpu_symbol_prog() {
	GLuint sidVert = load_shader("symbol.vert");
	GLuint sidFrag = load_shader("symbol.frag");
	s_progSymbol.init(sidVert, sidFrag);
	glDeleteShader(sidVert);
	glDeleteShader(sidFrag);
}

static void init_gpu_batch_progs() {
	GLuint sidVertStatic = load_shader("batch_static.vert");
	GLuint sidVertDeform = load_shader("batch_deform.vert");
	GLuint sidFrag = load_shader("batch.frag");
	s_progStatic.init(sidVertStatic, sidFrag);
	s_progDeform.init(sidVertDeform, sidFrag);
	glDeleteShader(sidVertStatic);
	glDeleteShader(sidVertDeform);
	glDeleteShader(sidFrag);
}

static void init_gpu() {
	init_gpu_symbol_prog();
	init_gpu_batch_progs();
	nxCore::dbg_msg("GPU programs:\n");
	nxCore::dbg_msg(" * symbol: %d\n", s_progSymbol.get_id());
	nxCore::dbg_msg(" * static: %d\n", s_progStatic.get_id());
	nxCore::dbg_msg(" * deform: %d\n", s_progDeform.get_id());
}

static void reset_gpu_progs() {
	s_progSymbol.reset();
	s_progStatic.reset();
	s_progDeform.reset();
}

static void reset_gpu() {
	reset_gpu_progs();
}

static void init_font() {
	// ...
}

static void reset_font() {
	if (s_fontIBO) {
		glDeleteBuffers(1, &s_fontIBO);
		s_fontIBO = 0;
	}
	if (s_fontVBO) {
		glDeleteBuffers(1, &s_fontVBO);
		s_fontVBO = 0;
	}
	s_pFont = nullptr;
}

static void init_impl(int shadowSize, cxResourceManager* pRsrcMgr, Draw::Font* pFont) {
	s_pRsrcMgr = pRsrcMgr;
	if (s_pRsrcMgr == nullptr) return;
	s_pFont = pFont;
	init_rsrc_mgr();
	init_font();
	init_gpu();
}

static void reset_impl() {
	reset_gpu();
}

static int get_screen_width_impl() {
	return OGLSys::get_width();
}

static int get_screen_height_impl() {
	return OGLSys::get_height();
}

static cxMtx get_shadow_bias_mtx_impl() {
	static const float bias[4 * 4] = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	};
	return nxMtx::from_mem(bias);
}

static void init_prims_impl(const uint32_t maxVtx, const uint32_t maxIdx) {
	// ...
}

static void prim_geom_impl(const Draw::PrimGeom* pGeom) {
	// ...
}

static void prim_impl(const Draw::Prim* pPrim, const Draw::Context* pCtx) {
	// ...
}

static void batch_impl(cxModelWork* pWk, const int ibat, const Draw::Mode mode, const Draw::Context* pCtx) {
	// ...
}

static void quad_impl(const Draw::Quad* pQuad) {
	// ...
}

void symbol_impl(const Draw::Symbol* pSym) {
	// ...
}

static void begin_impl(const cxColor& clearColor) {
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void end_impl() {
	OGLSys::swap();
}

static Draw::Ifc s_ifc;

struct DrwInit {
	DrwInit() {
		nxCore::mem_zero(&s_ifc, sizeof(s_ifc));
		s_ifc.info.pName = "simple_ogl";
		s_ifc.info.needOGLContext = true;
		s_ifc.init = init_impl;
		s_ifc.reset = reset_impl;
		s_ifc.get_screen_width = get_screen_width_impl;
		s_ifc.get_screen_height = get_screen_height_impl;
		s_ifc.get_shadow_bias_mtx = get_shadow_bias_mtx_impl;
		s_ifc.init_prims = init_prims_impl;
		s_ifc.prim_geom = prim_geom_impl;
		s_ifc.begin = begin_impl;
		s_ifc.end = end_impl;
		s_ifc.batch = batch_impl;
		s_ifc.prim = prim_impl;
		s_ifc.quad = quad_impl;
		s_ifc.symbol = symbol_impl;
		Draw::register_ifc_impl(&s_ifc);
	}
} s_drwInit;

DRW_IMPL_END
