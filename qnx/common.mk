#
# Copyright  2010, QNX Software Systems Ltd.  All Rights Reserved
#
# This source code has been published by QNX Software Systems Ltd.
# (QSSL).  However, any use, reproduction, modification, distribution
# or transfer of this software, or any software which includes or is
# based upon any of this code, is only permitted under the terms of
# the QNX Open Community License version 1.0 (see licensing.qnx.com for
# details) or as otherwise expressly authorized by a written license
# agreement from QSSL.  For more information, please email licensing@qnx.com.
#
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Quake3
endef

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)
USEFILE=$(PROJECT_ROOT)/quake3.use
NAME=quake3

Q3_SRC_ROOT = $(PRODUCT_ROOT)

CCFLAGS += -DRENDER_COLOUR_SOFTWARE -DSTATIC_LINK -DUSE_OPENGL_ES_1_1 -DSIGIOT=SIGABRT -DFNDELAY=O_NONBLOCK -DSIGTTIN=SIGSTOP
CCFLAGS += -D_LIB -DBOTLIB -DCGAME_HARD_LINKED -DGAME_HARD_LINKED -DUI_HARD_LINKED -DGLOBALRANK -DQ3_UI_EXPORT -DUSE_OPENAL

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH +=	$(PRODUCT_ROOT)/../openal/include  

#===== EXTRA_LIBVPATH - a space-separated list of directories to search for library files.
EXTRA_LIBVPATH +=	$(PRODUCT_ROOT)/../openal/qnx/arm/so.le.v7  

EXTRA_SRCVPATH += 	 %.c $(Q3_SRC_ROOT)/code/renderer
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/unix
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/null
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/q3_ui
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/game
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/ui
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/jpeg-6
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/cgame
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/client
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/qcommon
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/game
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/server
EXTRA_SRCVPATH +=    %.c $(Q3_SRC_ROOT)/code/botlib
EXTRA_SRCVPATH +=    %.cpp $(Q3_SRC_ROOT)/code/splines

SRCS	  =     tr_animation.c \
                tr_backend.c \
                tr_bsp.c \
                tr_cmds.c \
                tr_curve.c \
                tr_flares.c \
                tr_font.c \
                tr_image.c \
                tr_init.c \
                tr_light.c \
                tr_main.c \
                tr_marks.c \
                tr_mesh.c \
                tr_model.c \
                tr_noise.c \
                tr_scene.c \
                tr_shade.c \
                tr_shade_calc.c \
                tr_shader.c \
                tr_shadows.c \
                tr_sky.c \
                tr_surface.c \
                tr_world.c \
				ui_addbots.c \
                ui_atoms.c \
                ui_cdkey.c \
                ui_cinematics.c \
                ui_confirm.c \
                ui_connect.c \
                ui_controls2.c \
                ui_credits.c \
                ui_demo2.c \
                ui_display.c \
                ui_gameinfo.c \
                ui_ingame.c \
                ui_main.c \
                ui_menu.c \
                ui_mfield.c \
                ui_mods.c \
                ui_network.c \
                ui_options.c \
                ui_playermodel.c \
                ui_players.c \
                ui_playersettings.c \
                ui_preferences.c \
                ui_qmenu.c \
                ui_removebots.c \
                ui_serverinfo.c \
                ui_servers2.c \
                ui_setup.c \
                ui_sound.c \
                ui_sparena.c \
                ui_specifyserver.c \
                ui_splevel.c \
                ui_sppostgame.c \
                ui_spreset.c \
                ui_spskill.c \
                ui_startserver.c \
                ui_team.c \
                ui_teamorders.c \
                ui_video.c \
                ui_vkb \
                bg_misc.c \
                q_math.c \
                q_shared.c \
                ui_syscalls.c \
				jcapimin.c \
                jccoefct.c \
                jccolor.c \
                jcdctmgr.c \
                jchuff.c \
                jcinit.c \
                jcmainct.c \
                jcmarker.c \
                jcmaster.c \
                jcomapi.c \
                jcparam.c \
                jcphuff.c \
                jcprepct.c \
                jcsample.c \
                jctrans.c \
                jdapimin.c \
                jdapistd.c \
                jdatadst.c \
                jdatasrc.c \
                jdcoefct.c \
                jdcolor.c \
                jddctmgr.c \
                jdhuff.c \
                jdinput.c \
                jdmainct.c \
                jdmarker.c \
                jdmaster.c \
                jdmerge.c \
                jdphuff.c \
                jdpostct.c \
                jdsample.c \
                jdtrans.c \
                jerror.c \
                jfdctflt.c \
                jfdctfst.c \
                jfdctint.c \
                jidctflt.c \
                jidctfst.c \
                jidctint.c \
                jidctred.c \
                jmemansi.c \
                jmemmgr.c \
                jquant1.c \
                jquant2.c \
                jutils.c \
				ai_chat.c \
                ai_cmd.c \
                ai_dmnet.c \
                ai_dmq3.c \
                ai_main.c \
                ai_team.c \
                ai_vcmd.c \
                bg_misc.c \
                bg_pmove.c \
                bg_slidemove.c \
                g_active.c \
                g_arenas.c \
                g_bot.c \
                g_client.c \
                g_cmds.c \
                g_combat.c \
                g_items.c \
                g_main.c \
                g_mem.c \
                g_misc.c \
                g_missile.c \
                g_mover.c \
                g_session.c \
                g_spawn.c \
                g_svcmds.c \
                g_syscalls.c \
                g_target.c \
                g_team.c \
                g_trigger.c \
                g_utils.c \
                g_weapon.c \
                q_math.c \
                q_shared.c \
				cg_consolecmds.c \
                cg_draw.c \
                cg_drawtools.c \
                cg_effects.c \
                cg_ents.c \
                cg_event.c \
                cg_info.c \
                cg_localents.c \
                cg_main.c \
                cg_marks.c \
                cg_players.c \
                cg_playerstate.c \
                cg_predict.c \
                cg_scoreboard.c \
                cg_servercmds.c \
                cg_snapshot.c \
                cg_syscalls.c \
                cg_view.c \
                cg_weapons.c \
                bg_misc.c \
                bg_pmove.c \
                bg_slidemove.c \
                q_math.c \
                q_shared.c \
                ui_shared.c \
				be_aas_bspq3.c \
                be_aas_cluster.c \
                be_aas_debug.c \
                be_aas_entity.c \
                be_aas_file.c \
                be_aas_main.c \
                be_aas_move.c \
                be_aas_optimize.c \
                be_aas_reach.c \
                be_aas_routealt.c \
                be_aas_route.c \
                be_aas_sample.c \
                be_ai_char.c \
                be_ai_chat.c \
                be_ai_gen.c \
                be_ai_goal.c \
                be_ai_move.c \
                be_ai_weap.c \
                be_ai_weight.c \
                be_ea.c \
                be_interface.c \
                l_crc.c \
                l_libvar.c \
                l_log.c \
                l_memory.c \
                l_precomp.c \
                l_script.c \
                l_struct.c \
				math_angles.cpp \
                math_matrix.cpp \
                math_quaternion.cpp \
                math_vector.cpp \
                q_parse.cpp \
                q_shared.cpp \
                splines.cpp \
                util_str.cpp \
				cl_cgame.c \
                cl_cin.c \
                cl_console.c \
                cl_keys.c \
                cl_main.c \
                cl_net_chan.c \
                cl_parse.c \
                cl_scrn.c \
                cl_ui.c \
				qal.c \
				snd_adpcm.c \
                snd_codec.c \
                snd_codec_wav.c \
                snd_dma.c \
                snd_main.c \
                snd_mem.c \
                snd_mix.c \
                snd_openal.c \
                snd_wavelet.c \
				null_snddma.c \
                cmd.c \
                cm_load.c \
                cm_patch.c \
                cm_polylib.c \
                cm_test.c \
                cm_trace.c \
                common.c \
                cvar.c \
                files.c \
                huffman.c \
                md4.c \
                msg.c \
                net_chan.c \
                sv_bot.c \
                sv_ccmds.c \
                sv_client.c \
                sv_game.c \
                sv_init.c \
                sv_main.c \
                sv_net_chan.c \
                sv_snapshot.c \
                sv_world.c \
                unix_shared.c \
                unzip.c \
                vm.c \
                vm_interpreted.c \
                vm_arm.c \
                qnx_main.c \
                qnx_net.c \
                qnx_glimp.c \
				cl_input.c \
                linux_signals.c \
                q_math.c \
                q_shared.c

include $(MKFILES_ROOT)/qmacros.mk

include $(MKFILES_ROOT)/qtargets.mk


#===== LIBS - a space-separated list of library items to be included in the link.
ifeq ($(CPU), arm)
	ifneq ($(filter v7, $(VARIANT_LIST)), v7)
		GCCVER:= $(if $(GCC_VERSION), $(GCC_VERSION), $(shell qcc -V 2>&1 | grep default | sed -e 's/,.*//'))
		ifneq ($(filter 4.%, $(strip $(GCCVER))),)
			CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
			LIBS += m-vfp
		else
			LIBS += m
		endif
	else
		LIBS += m
	endif
else
	LIBS += m
endif

LIBS += EGL GLESv1_CM screen socket pps asound openal
