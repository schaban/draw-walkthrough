#!/bin/sh

PROR_NAME=draw_prog

PROG_DIR=prog
CORE_DIR=core
DATA_DIR=data

INC_DIR=inc
SRC_DIR=src

SHADERS_SRC_DIR=$SRC_DIR/shaders
SHADERS_TGT_DIR=$DATA_DIR/simple_ogl

PROG_PATH=$PROG_DIR/$PROR_NAME
RUN_PATH=run.sh

FMT_BOLD="\e[1m"
FMT_UNDER="\e[4m"
FMT_BLACK="\e[30m"
FMT_BLACK_BG="\e[30m"
FMT_RED="\e[31m"
FMT_RED_BG="\e[41m"
FMT_GREEN="\e[32m"
FMT_GREEN_BG="\e[42m"
FMT_YELLOW="\e[33m"
FMT_YELLOW_BG="\e[43m"
FMT_BLUE="\e[34m"
FMT_BLUE_BG="\e[44m"
FMT_MAGENTA="\e[35m"
FMT_MAGENTA_BG="\e[45m"
FMT_CYAN="\e[36m"
FMT_CYAN_BG="\e[46m"
FMT_WHITE="\e[37m"
FMT_WHITE_BG="\e[47m"
FMT_GRAY="\e[90m"
FMT_GRAY_BG="\e[100m"
FMT_B_RED="\e[91m"
FMT_B_RED_BG="\e[101m"
FMT_B_GREEN="\e[92m"
FMT_B_GREEN_BG="\e[102m"
FMT_B_YELLOW="\e[93m"
FMT_B_YELLOW_BG="\e[103m"
FMT_B_BLUE="\e[94m"
FMT_B_BLUE_BG="\e[104m"
FMT_B_MAGENTA="\e[95m"
FMT_B_MAGENTA_BG="\e[105m"
FMT_B_CYAN="\e[96m"
FMT_B_CYAN_BG="\e[106m"
FMT_B_WHITE="\e[97m"
FMT_B_WHITE_BG="\e[107m"
FMT_OFF="\e[0m"

printf "$FMT_MAGENTA$FMT_BOLD""/------------------------------------------\ ""$FMT_OFF\n"
printf " ""$FMT_BLUE_BG$FMT_B_YELLOW"" .: Compiling draw-walkthrough project :. ""$FMT_OFF\n"
printf "$FMT_MAGENTA$FMT_BOLD""\------------------------------------------/ ""$FMT_OFF\n"

if [ ! -d $PROG_DIR ]; then
	mkdir -p $PROG_DIR
fi

CORE_SRC_URL="https://raw.githubusercontent.com/schaban/crosscore_dev/main/src"
CORE_SRCS="crosscore.hpp crosscore.cpp demo.hpp demo.cpp draw.hpp oglsys.hpp oglsys.cpp oglsys.inc scene.hpp scene.cpp smprig.hpp smprig.cpp draw_ogl.cpp main.cpp"
CORE_OGL_SRCS="gpu_defs.h progs.inc shaders.inc"

NEED_CORE=0
if [ ! -d $CORE_DIR ]; then
	mkdir -p $CORE_DIR
	mkdir -p $CORE_DIR/ogl
	NEED_CORE=1
else
	for src in $CORE_SRCS; do
		if [ $NEED_CORE -ne 1 ]; then
			if [ ! -f $CORE_DIR/$src ]; then
				NEED_CORE=1
			fi
		fi
	done
	if [ ! -d $CORE_DIR/ogl ]; then
		mkdir -p $CORE_DIR/ogl
		NEED_CORE=1
	fi
	if [ $NEED_CORE -ne 1 ]; then
		for src in $CORE_OGL_SRCS; do
			if [ ! -f $CORE_DIR/ogl/$src ]; then
				NEED_CORE=1
			fi
		done
	fi
fi

if [ $NEED_CORE -ne 0 ]; then
	printf "$FMT_B_RED""-> Downloading crosscore sources...""$FMT_OFF\n"
	for src in $CORE_SRCS; do
		if [ ! -f $CORE_DIR/$src ]; then
			printf "$FMT_B_BLUE""     $src""$FMT_OFF\n"
			curl -o $CORE_DIR/$src $CORE_SRC_URL/$src
		fi
	done
	for src in $CORE_OGL_SRCS; do
		if [ ! -f $CORE_DIR/ogl/$src ]; then
			printf "$FMT_CYAN""     $src""$FMT_OFF\n"
			curl -o $CORE_DIR/ogl/$src $CORE_SRC_URL/ogl/$src
		fi
	done
fi

KHR_REG_URL=https://registry.khronos.org
INC_OGL=$INC_DIR/GL
if [ ! -d $INC_OGL ]; then
	mkdir -p $INC_OGL
fi
OGL_INC_DL=0
for kh in glext.h glcorearb.h wglext.h; do
	if [ ! -f $INC_OGL/$kh ]; then
		if [ $OGL_INC_DL -ne 1 ]; then
			printf "$FMT_OFF""-> Downloading OpenGL headers...\n"
			OGL_INC_DL=1
		fi
		printf "$FMT_B_GREEN""     $kh""$FMT_OFF\n"
		curl -o $INC_OGL/$kh $KHR_REG_URL/OpenGL/api/GL/$kh
	fi
done
INC_KHR=$INC_DIR/KHR
if [ ! -d $INC_KHR ]; then
	mkdir -p $INC_KHR
fi
for kh in khrplatform.h; do
	if [ ! -f $INC_KHR/$kh ]; then
		if [ $OGL_INC_DL -ne 1 ]; then
			printf "$FMT_OFF""-> Downloading OpenGL headers...\n"
			OGL_INC_DL=1
		fi
		printf "$FMT_B_GREEN""     $kh""$FMT_OFF\n"
		curl -o $INC_KHR/$kh $KHR_REG_URL/EGL/api/KHR/$kh
	fi
done

INCS="-I $CORE_DIR -I $INC_DIR "
SRCS="`ls $SRC_DIR/*.cpp` `ls $CORE_DIR/*.cpp`"

DEFS="-DX11"
LIBS="-lX11 -lpthread"

LIBS="$LIBS -ldl"
CXX=${CXX:-g++}

printf "Compiling \"$FMT_BOLD$FMT_B_MAGENTA$PROG_PATH$FMT_OFF\" with $FMT_BOLD$CXX$FMT_OFF.\n"
rm -f $PROG_PATH
rm -f $RUN_PATH
$CXX -std=c++11 -ggdb $DEFS $INCS $SRCS -o $PROG_PATH $LIBS $*
if [ -f "$PROG_PATH" ]; then
	if [ ! -d $SHADERS_TGT_DIR ]; then
		mkdir -p $SHADERS_TGT_DIR
	fi
	printf "$FMT_OFF""Copying GLSL code:\n""$FMT_GRAY"
	for glsl in symbol.vert symbol.frag batch_static.vert batch_deform.vert batch.frag; do
		cp -v $SHADERS_SRC_DIR/$glsl $SHADERS_TGT_DIR/$glsl
	done
	printf "$FMT_OFF""Done!\n"
	echo "#!/bin/sh\n" > $RUN_PATH
	echo "./$PROG_PATH \$*" >> $RUN_PATH
	chmod +x $RUN_PATH
	printf "$FMT_B_GREEN""Success""$FMT_OFF""$FMT_BOLD""!!""$FMT_OFF\n"
else
	printf "$FMT_B_RED""Failure""$FMT_OFF...\n"
fi

