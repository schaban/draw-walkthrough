#include "crosscore.hpp"
#include "scene.hpp"
#include "smprig.hpp"
#include "demo.hpp"

DEMO_PROG_BEGIN

#define STG_NAME "fountain"


static int s_mode = 0;
static float s_csclChr = 1.0f;
static float s_csclStg = 1.0f;
static float s_csclTxt = 1.0f;

static float get_mot_speed() {
	return 1.0f;
}


static void chr_ground_adj(ScnObj* pObj, sxCollisionData* pCol) {
	if (!pObj) return;
	if (!pCol) return;
	cxVec wpos = pObj->get_world_pos();
	float y = Scene::get_ground_height(pCol, wpos);
	pObj->set_skel_root_local_ty(y);
}

static void chr_wall_adj(ScnObj* pObj, sxCollisionData* pCol) {
	if (!pObj) return;
	if (!pCol) return;
	cxVec wpos = pObj->get_world_pos();
	cxVec npos = wpos;
	cxVec opos = pObj->get_prev_world_pos();
	cxVec apos = wpos;
	if (pObj->mObjAdjYOffs == 0.0f) {
		pObj->mObjAdjYOffs = 1.0f;
	}
	float yoffs = pObj->mObjAdjYOffs;
	npos.y += yoffs;
	opos.y += yoffs;
	if (pObj->mObjAdjRadius == 0.0f) {
		pObj->mObjAdjRadius = 0.42f;
	}
	float radius = pObj->mObjAdjRadius;
	bool adjFlg = Scene::wall_adj(pObj->mpJobCtx, pCol, npos, opos, radius, &apos);
	if (adjFlg) {
		cxVec prevPos(opos.x, wpos.y, opos.z);
		cxVec pos(apos.x, wpos.y, apos.z);
		if (Scene::get_frame_count() > 1) {
			float dist = nxVec::dist(wpos, pos);
			if (dist > 0.1f) {
				pos.lerp(opos, pos, 0.25f);
				pos.y = wpos.y;
			}
		}
		pos = nxCalc::approach(prevPos, pos, 10);
		pObj->set_skel_root_local_pos(pos.x, pos.y, pos.z);
	}
}

static void chr_before_blend(ScnObj* pObj) {
	if (!pObj) return;
	if (!pObj) return;
	Pkg* pStgPkg = Scene::find_pkg(STG_NAME);
	if (!pStgPkg) return;
	sxCollisionData* pCol = pStgPkg->find_collision("col");
	chr_ground_adj(pObj, pCol);
	chr_wall_adj(pObj, pCol);
	SmpRig* pRig = pObj->get_ptr_wk<SmpRig>(0);
	if (pRig) {
		pRig->exec(pCol);
	}
}

//////////////////////////////////////////////////////////////////////////
static void bishojo_bat_pre_draw(ScnObj* pObj, const int ibat) {
	if (!pObj) return;
	Scene::push_ctx();
	const char* pMtlName = pObj->get_batch_mtl_name(ibat);
	cxColor baseClrScl(1.0f);
	if (pMtlName && nxCore::str_eq(pMtlName, "hair")) {
		baseClrScl.set(1.75f, 1.6f, 1.95f);
	} else {
		baseClrScl.set(0.52f, 0.58f, 0.68f);
	}
	baseClrScl.scl(s_csclChr);
	pObj->set_base_color_scl(baseClrScl);
	if (pMtlName && nxCore::str_eq(pMtlName, "head")) {
		Scene::set_hemi_exp(0.5f);
	} else {
		Scene::set_hemi_exp(0.75f);
	}
	Scene::set_gamma_rgb(1.6f, 1.6f, 1.4f);
}

static void bishojo_bat_post_draw(ScnObj* pObj, const int ibat) {
	if (!pObj) return;
	Scene::pop_ctx();
}

static void bishojo_init(ScnObj* pObj) {
	if (!pObj) return;
	pObj->set_shadow_weight_bias(200.0f);
	pObj->set_shadow_density_scl(0.75f);
	pObj->set_base_color_scl(0.5f);

	pObj->mBatchPreDrawFunc = bishojo_bat_pre_draw;
	pObj->mBatchPostDrawFunc = bishojo_bat_post_draw;

	pObj->mBeforeBlendFunc = chr_before_blend;

	if (s_mode == 1) {
		pObj->set_world_quat_pos(nxQuat::from_degrees(0.0f, 40.0f, 0.0f), cxVec(1.2f, 0.0f, 2.5f));
	} else {
		//pObj->set_world_quat_pos(nxQuat::from_degrees(0.0f, 0.0f, 0.0f), cxVec(2.0f, 0.0f, 2.0f));
		//pObj->set_world_quat_pos(nxQuat::from_degrees(0.0f, -60.0f, 0.0f), cxVec(2.0f, 0.0f, 4.0f));
		//pObj->set_world_quat_pos(nxQuat::from_degrees(0.0f, -50.0f, 0.0f), cxVec(2.0f, 0.0f, 2.0f));
		pObj->set_world_quat_pos(nxQuat::from_degrees(0.0f, 60.0f, 0.0f), cxVec(1.53f, 0.0f, -2.5f));
	}

	sxValuesData* pVals = pObj->find_values("params");
	if (pVals) {
		SmpRig* pRig = nxCore::tMem<SmpRig>::alloc(1, "SmpRig");
		if (pRig) {
			pObj->set_ptr_wk(0, pRig);
			pRig->init(pObj, pVals);
		}
	}
}

static void bishojo_exec(ScnObj* pObj) {
	if (!pObj) return;
	float mspeed = get_mot_speed() * 0.5f;
	sxMotionData* pMot = pObj->find_motion(s_mode == 1 ? "stand" : "walk");
	pObj->move(pMot, mspeed * Scene::speed());
	if (s_mode != 1)  {
		float addDY = 0.85f;
		pObj->add_world_deg_y(-addDY * mspeed * Scene::speed());
	}
}

static void bishojo_del(ScnObj* pObj) {
	if (!pObj) return;
	SmpRig* pRig = pObj->get_ptr_wk<SmpRig>(0);
	if (pRig) {
		nxCore::tMem<SmpRig>::free(pRig);
		pObj->set_ptr_wk<void>(0, nullptr);
	}
}


static void init() {
	s_mode = nxApp::get_int_opt("mode", 0);
	s_csclChr = nxApp::get_float_opt("cscl_chr", 1.0f);
	s_csclStg = nxApp::get_float_opt("cscl_stg", 1.0f);
	s_csclTxt = nxApp::get_float_opt("cscl_txt", 1.0f);

	DEMO_ADD_CHR(bishojo);

	Scene::add_obj(STG_NAME);
	ScnObj* pStgObj = Scene::find_obj(STG_NAME);
	if (pStgObj) {
		pStgObj->mDisableShadowCast = true;
		cxColor baseScl(1.5f, 1.4f, 1.2f);
		baseScl.scl(s_csclStg);
		pStgObj->set_base_color_scl(baseScl);
	}

	cxResourceManager* pRsrcMgr = Scene::get_rsrc_mgr();
	if (pRsrcMgr) {
		pRsrcMgr->prepare_all_gfx();
	}
}

static void view_exec() {
	static int tgtCnt = 0;
	static float xoffs = 0.0f;
	static float zoffs = 0.0f;
	static float addx = 0.001f;
	static float addz = 0.001f;
	static float fovMin = 20.0f;
	static float fovMax = 30.0f;
	static float fovAdd = 0.1f;
	static float fov = fovMin;
	++tgtCnt;
	xoffs += addx;
	zoffs += addz;
	if (tgtCnt > 800) {
		tgtCnt = 0;
		addx = -addx;
		addz = -addz;
	}
	ScnObj* pTgtObj = Scene::find_obj("bishojo");
	if (!pTgtObj) return;
	cxVec tgt = pTgtObj->get_center_pos();
	tgt.y += nxCalc::fit(fov, fovMin, fovMax, 0.3f, 0.0f);
	static float ry = 0.0f;
	cxQuat q = nxQuat::from_degrees(3, ry, 0);
	//cxVec offs = q.apply(cxVec(0.0f, -0.1f, 4.0f));
	cxVec offs = q.apply(cxVec(xoffs, 0.1f, 3.0f + zoffs));
	cxVec pos = tgt + offs;
	static cxVec prevPos;
	static cxVec prevTgt;
	if (Scene::get_frame_count() > 0) {
		pos = nxCalc::approach(prevPos, pos, 50);
		tgt = nxCalc::approach(prevPos, tgt, 20);
	}
	prevPos = pos;
	prevTgt = tgt;
	Scene::set_view(pos, tgt);
	Scene::set_deg_FOVY(fov);
	if (fov < fovMax) {
		fov += fovAdd;
	} else {
		fov = fovMax;
	}
}

static void set_fog() {
	Scene::set_fog_rgb(0.74f, 0.75f, 0.54f);
	Scene::set_fog_density(1.0f);
	Scene::set_fog_range(10.0f, 1000.0f);
	Scene::set_fog_curve(0.1f, 0.1f);
}

static void set_cc() {
	Scene::reset_color_correction();

	Scene::set_linear_white_rgb(0.9f, 0.9f, 0.99f);
	Scene::set_linear_gain(1.2f);
	Scene::set_linear_bias(0.0f);

	Scene::set_gamma_rgb(2.3f, 2.42f, 2.3f);
	Scene::set_exposure_rgb(4.15f, 5.0f, 4.15f);
}

static void set_scene_ctx() {
	//Scene::set_shadow_uniform(false);//

	Scene::set_shadow_density(1.0f);
	Scene::set_shadow_fade(35.0f, 40.0f);
	Scene::set_shadow_proj_params(40.0f, 40.0f, 100);
	Scene::set_shadow_dir_degrees(50.0f, 110.0f);
	Scene::set_spec_dir_to_shadow();
	Scene::set_spec_shadowing(0.9f);

	//
	Scene::set_hemi_upper(1.1f, 1.18f, 1.12f);
	Scene::set_hemi_lower(0.22f, 0.16f, 0.12f);
	cxVec up = cxVec(0.0f, 1.0f, 0.0f);
	Scene::set_hemi_up(up);
	Scene::set_hemi_exp(1.25f);
	Scene::set_hemi_gain(1.0f);

	set_fog();
	set_cc();
}

static void draw_msg(
	const char* pMsg,
	const float msgOX, const float msgOY,
	const float fontW, const float fontH,
	const cxColor& msgClr
) {
	char msgBuf[256];
	if (!pMsg) return;
	Scene::set_font_size(fontW, fontH);
	float msgX = msgOX;
	float msgY = msgOY;
	const char* pMsgSrc = pMsg;
	char* pMsgBuf = msgBuf;
	size_t msgLen = 0;
	bool emitFlg = false;
	bool endFlg = false;
	bool nextLine = false;
	while (true) {
		char c = *pMsgSrc++;
		if (c == 0) {
			emitFlg = true;
			endFlg = true;
		} else if (c == 0xA) {
			nextLine = true;
			emitFlg = true;
		} else {
			if (msgLen < sizeof(msgBuf) - 1) {
				pMsgBuf[msgLen] = c;
				++msgLen;
			}
		}
		if (emitFlg) {
			if (msgLen > 0) {
				pMsgBuf[msgLen] = 0;
				Scene::symbol_str(pMsgBuf, msgX, msgY, msgClr);
				msgLen = 0;
			}
			emitFlg = false;
		}
		if (endFlg) {
			break;
		}
		if (nextLine) {
			msgY -= fontH + 4.0f;
			nextLine = false;
		}
	}
}

static void draw_2d() {
	float refSizeX = 800;
	float refSizeY = 800;
	Scene::set_ref_scr_size(refSizeX, refSizeY);
	if (Scene::get_view_mode_width() < Scene::get_view_mode_height()) {
		Scene::set_ref_scr_size(refSizeY, refSizeX);
	}

	const char* pText = "* DRAW-WALKTHROUGH *";
	float fontW = 40;
	float fontH = 50;
	float x = 10;
	float y = -fontH;
	static float dx = 0.0f;
	static float dy = 0.0f;
	x += dx;
	y += dy;
	static float alpha = 0.9f;
	if (alpha > 0.0f) {
		cxColor c1(0.1f, 0.31f, 0.1f, alpha * 0.5f);
		cxColor c2(0.2f, 0.51f, 0.1f, alpha * 0.5f);
		cxColor c0(0.95f, 0.5f, 0.25f, alpha);
		c0.scl_rgb(s_csclTxt);
		c1.scl_rgb(s_csclTxt);
		c2.scl_rgb(s_csclTxt);
		draw_msg(pText, x - 1, y + 1, fontW, fontH, c1);
		draw_msg(pText, x + 1, y - 1, fontW, fontH, c2);
		draw_msg(pText, x, y, fontW, fontH, c0);
		if (Scene::get_frame_count() > 320) {
			alpha -= 0.01f;
		}
		if (s_mode == 1)  {
			dx += 0.05f;
			dy += 0.75f;
		} else {
			dx += 0.125f;
			dy += 0.5f;
		}
	}
}


static void loop(void* pLoopCtx) {
	set_scene_ctx();
	Scene::exec();
	view_exec();
	Scene::visibility();
	cxColor bgClr(0.21f, 0.31f, 0.48f);
	bgClr.to_nonlinear();
	Scene::frame_begin(bgClr);
	Scene::draw();
	draw_2d();
	Scene::frame_end();
}

static void reset() {
	// ...
}

DEMO_REGISTER(default);

DEMO_PROG_END
