#include <ui_local.h>



// VKB related variables defined here.
int vkb_m_keyboardControlFD = -1;
pps_encoder_t vkb_m_encoder;
pps_decoder_t vkb_m_decoder;
char* vkb_gKeyboardControlPath = "/pps/services/input/control?wait";
bool visible = false;


void VKB_ShowKeyboard()
{
	if (vkb_m_keyboardControlFD == -1)
	{
		vkb_m_keyboardControlFD = open( vkb_gKeyboardControlPath, O_RDWR );
		pps_encoder_initialize( &vkb_m_encoder, false );
		pps_decoder_initialize( &vkb_m_decoder, false );
	}

	pps_encoder_reset( &vkb_m_encoder );
	pps_encoder_add_string( &vkb_m_encoder, "msg", "show" );
	write( vkb_m_keyboardControlFD, pps_encoder_buffer( &vkb_m_encoder ), pps_encoder_length( &vkb_m_encoder ) );
}


void VKB_HideKeyboard()
{
	if (vkb_m_keyboardControlFD != -1)
	{
		pps_encoder_reset( &vkb_m_encoder );
		pps_encoder_add_string( &vkb_m_encoder, "msg", "hide" );
		write( vkb_m_keyboardControlFD, pps_encoder_buffer(&vkb_m_encoder), pps_encoder_length(&vkb_m_encoder));
	}
}
