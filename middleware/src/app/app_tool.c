
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "app_include.h"
typedef struct iso639_lang
{
	const char  *psz_eng_name;        /* Description in English */
	const char  *psz_native_name;     /* Description in native language */
	const char  *psz_iso639_1;        /* ISO-639-1 (2 characters code */
	const char  *psz_iso639_2T;       /* ISO-639-2/T (3 characters English code */
	const char  *psz_iso639_2B;       /* ISO-639-2/B (3 characters native code */
}iso639_lang_t;

static const iso639_lang_t p_languages[] =
{
	{   "Afar" ,             "",            "aa", "aar", "aar" },
	{   "Abkhazian" ,        "",            "ab", "abk", "abk" },
	{   "Afrikaans" ,        "",            "af", "afr", "afr" },
	{   "Albanian" ,         "",            "sq", "sqi", "alb" },
	{   "Amharic" ,          "",            "am", "amh", "amh" },
	{   "Arabic" ,           "????",        "ar", "ara", "ara" },
	{   "Armenian" ,         "",            "hy", "hye", "arm" },
	{   "Assamese" ,         "",            "as", "asm", "asm" },
	{   "Avestan" ,          "",            "ae", "ave", "ave" },
	{   "Aymara" ,           "",            "ay", "aym", "aym" },
	{   "Azerbaijani" ,      "",            "az", "aze", "aze" },
	{   "Bashkir" ,          "",            "ba", "bak", "bak" },
	{   "Basque" ,           "",            "eu", "eus", "baq" },
	{   "Belarusian" ,       "",            "be", "bel", "bel" },
	{   "Bengali" ,          "?????",         "bn", "ben", "ben" },
	{   "Bihari" ,           "",            "bh", "bih", "bih" },
	{   "Bislama" ,          "",            "bi", "bis", "bis" },
	{   "Bosnian" ,          "",            "bs", "bos", "bos" },
	{   "Breton" ,           "",            "br", "bre", "bre" },
	{   "Bulgarian" ,        "български език", "bg", "bul", "bul" },
	{   "Burmese" ,          "",            "my", "mya", "bur" },
	{   "Catalan" ,          "Català",      "ca", "cat", "cat" },
	{   "Chamorro" ,         "",            "ch", "cha", "cha" },
	{   "Chechen" ,          "",            "ce", "che", "che" },
	{   "Chinese" ,          "简体中文",    "zh", "zho", "chi" },
	{   "Church Slavic" ,    "",            "cu", "chu", "chu" },
	{   "Chuvash" ,          "",            "cv", "chv", "chv" },
	{   "Cornish" ,          "",            "kw", "cor", "cor" },
	{   "Corsican" ,         "",            "co", "cos", "cos" },
	{   "Czech" ,            "?e?tina",     "cs", "ces", "cze" },
	{   "Danish" ,           "Dansk",       "da", "dan", "dan" },
	{   "Dutch" ,            "Nederlands",  "nl", "nld", "dut" },
	{   "Dzongkha" ,         "",            "dz", "dzo", "dzo" },
	{   "English" ,          "English",     "en", "eng", "eng" },
	{   "Esperanto" ,        "",            "eo", "epo", "epo" },
	{   "Estonian" ,         "",            "et", "est", "est" },
	{   "Faroese" ,          "",            "fo", "fao", "fao" },
	{   "Fijian" ,           "",            "fj", "fij", "fij" },
	{   "Finnish" ,          "Suomi",       "fi", "fin", "fin" },
	{   "French" ,           "Fran?ais",    "fr", "fra", "fre" },
	{   "Frisian" ,          "",            "fy", "fry", "fry" },
	{   "Georgian" ,         "???????",     "ka", "kat", "geo" },
	{   "German" ,           "Deutsch",     "de", "deu", "ger" },
	{   "Gaelic (Scots" ,   "",            "gd", "gla", "gla" },
	{   "Irish" ,            "",            "ga", "gle", "gle" },
	{   "Gallegan" ,         "Galego",      "gl", "glg", "glg" },
	{   "Manx" ,             "",            "gv", "glv", "glv" },
	{   "Greek, Modern (" , "",            "el", "gre", "ell" },
	{   "Guarani" ,          "",            "gn", "grn", "grn" },
	{   "Gujarati" ,         "",            "gu", "guj", "guj" },
	{   "Hebrew" ,           "?????",       "he", "heb", "heb" },
	{   "Herero" ,           "",            "hz", "her", "her" },
	{   "Hindi" ,            "",            "hi", "hin", "hin" },
	{   "Hiri Motu" ,        "",            "ho", "hmo", "hmo" },
	{   "Hungarian" ,        "Magyar",      "hu", "hun", "hun" },
	{   "Icelandic" ,        "Islenska",    "is", "isl", "ice" },
	{   "Inuktitut" ,        "",            "iu", "iku", "iku" },
	{   "Interlingue" ,      "",            "ie", "ile", "ile" },
	{   "Interlingua" ,      "",            "ia", "ina", "ina" },
	{   "Indonesian" ,       "Bahasa Indonesia", "id", "ind", "ind" },
	{   "Inupiaq" ,          "",            "ik", "ipk", "ipk" },
	{   "Italian" ,          "Italiano",    "it", "ita", "ita" },
	{   "Javanese" ,         "",            "jv", "jaw", "jav" },
	{   "Japanese" ,         "日本語",      "ja", "jpn", "jpn" },
	{   "Kalaallisut (Greenlandic" , "",   "kl", "kal", "kal" },
	{   "Kannada" ,          "",            "kn", "kan", "kan" },
	{   "Kashmiri" ,         "",            "ks", "kas", "kas" },
	{   "Kazakh" ,           "",            "kk", "kaz", "kaz" },
	{   "Khmer" ,            "",            "km", "khm", "khm" },
	{   "Kikuyu" ,           "",            "ki", "kik", "kik" },
	{   "Kinyarwanda" ,      "",            "rw", "kin", "kin" },
	{   "Kirghiz" ,          "",            "ky", "kir", "kir" },
	{   "Komi" ,             "",            "kv", "kom", "kom" },
	{   "Korean" ,           "???",      "ko", "kor", "kor" },
	{   "Kuanyama" ,         "",            "kj", "kua", "kua" },
	{   "Kurdish" ,          "",            "ku", "kur", "kur" },
	{   "Lao" ,              "",            "lo", "lao", "lao" },
	{   "Latin" ,            "",            "la", "lat", "lat" },
	{   "Latvian" ,          "",            "lv", "lav", "lav" },
	{   "Lingala" ,          "",            "ln", "lin", "lin" },
	{   "Lithuanian" ,       "",            "lt", "lit", "lit" },
	{   "Letzeburgesch" ,    "",            "lb", "ltz", "ltz" },
	{   "Macedonian" ,       "",            "mk", "mkd", "mac" },
	{   "Marshall" ,         "",            "mh", "mah", "mah" },
	{   "Malayalam" ,        "",            "ml", "mal", "mal" },
	{   "Maori" ,            "",            "mi", "mri", "mao" },
	{   "Marathi" ,          "",            "mr", "mar", "mar" },
	{   "Malay" ,            "Melayu",      "ms", "msa", "may" },
	{   "Malagasy" ,         "",            "mg", "mlg", "mlg" },
	{   "Maltese" ,          "",            "mt", "mlt", "mlt" },
	{   "Moldavian" ,        "",            "mo", "mol", "mol" },
	{   "Mongolian" ,        "",            "mn", "mon", "mon" },
	{   "Nauru" ,            "",            "na", "nau", "nau" },
	{   "Navajo" ,           "",            "nv", "nav", "nav" },
	{   "Ndebele, South" ,   "",            "nr", "nbl", "nbl" },
	{   "Ndebele, North" ,   "",            "nd", "nde", "nde" },
	{   "Ndonga" ,           "",            "ng", "ndo", "ndo" },
	{   "Nepali" ,           "",            "ne", "nep", "nep" },
	{   "Norwegian" ,        "Norsk",       "no", "nor", "nor" },
	{   "Norwegian Nynorsk" , "",           "nn", "nno", "nno" },
	{   "Norwegian Bokmaal" , "",           "nb", "nob", "nob" },
	{   "Chichewa; Nyanja" , "",            "ny", "nya", "nya" },
	{   "Occitan;Provencal" , "Occitan", "oc", "oci", "oci" },
	{   "Oriya" ,            "",            "or", "ori", "ori" },
	{   "Oromo" ,            "",            "om", "orm", "orm" },
	{   "Ossetian; Ossetic" , "",           "os", "oss", "oss" },
	{   "Panjabi" ,          "?????",         "pa", "pan", "pan" },
	{   "Persian" ,          "?????",       "fa", "fas", "per" },
	{   "Pali" ,             "",            "pi", "pli", "pli" },
	{   "Polish" ,           "Polski",      "pl", "pol", "pol" },
	{   "Portuguese" ,       "Português",   "pt", "por", "por" },
	{   "Pushto" ,           "",            "ps", "pus", "pus" },
	{   "Quechua" ,          "",            "qu", "que", "que" },
	{   "Original audio" ,   "",            "",   "qaa", "qaa" },
	{   "Raeto-Romance" ,    "",            "rm", "roh", "roh" },
	{   "Romanian" ,         "Roman?",      "ro", "ron", "rum" },
	{   "Rundi" ,            "",            "rn", "run", "run" },
	{   "Russian" ,          "Русский",     "ru", "rus", "rus" },
	{   "Sango" ,            "",            "sg", "sag", "sag" },
	{   "Sanskrit" ,         "",            "sa", "san", "san" },
	{   "Serbian" ,          "српски",      "sr", "srp", "scc" },
	{   "Croatian" ,         "Hrvatski",    "hr", "hrv", "scr" },
	{   "Sinhalese" ,        "",            "si", "sin", "sin" },
	{   "Slovak" ,           "Slovensky",   "sk", "slk", "slo" },
	{   "Slovenian" ,        "sloven??ina", "sl", "slv", "slv" },
	{   "Northern Sami" ,    "",            "se", "sme", "sme" },
	{   "Samoan" ,           "",            "sm", "smo", "smo" },
	{   "Shona" ,            "",            "sn", "sna", "sna" },
	{   "Sindhi" ,           "",            "sd", "snd", "snd" },
	{   "Somali" ,           "",            "so", "som", "som" },
	{   "Sotho, Southern" ,  "",            "st", "sot", "sot" },
	{   "Spanish" ,          "Espa?ol",     "es", "spa", "spa" },
	{   "Sardinian" ,        "",            "sc", "srd", "srd" },
	{   "Swati" ,            "",            "ss", "ssw", "ssw" },
	{   "Sundanese" ,        "",            "su", "sun", "sun" },
	{   "Swahili" ,          "",            "sw", "swa", "swa" },
	{   "Swedish" ,          "Svenska",     "sv", "swe", "swe" },
	{   "Tahitian" ,         "",            "ty", "tah", "tah" },
	{   "Tamil" ,            "",            "ta", "tam", "tam" },
	{   "Tatar" ,            "",            "tt", "tat", "tat" },
	{   "Telugu" ,           "",            "te", "tel", "tel" },
	{   "Tajik" ,            "",            "tg", "tgk", "tgk" },
	{   "Tagalog" ,          "",            "tl", "tgl", "tgl" },
	{   "Thai" ,             "",            "th", "tha", "tha" },
	{   "Tibetan" ,          "",            "bo", "bod", "tib" },
	{   "Tigrinya" ,         "",            "ti", "tir", "tir" },
	{   "Tonga (Tonga Islands" , "",       "to", "ton", "ton" },
	{   "Tswana" ,           "",            "tn", "tsn", "tsn" },
	{   "Tsonga" ,           "",            "ts", "tso", "tso" },
	{   "Turkish" ,          "Türk?e",      "tr", "tur", "tur" },
	{   "Turkmen" ,          "",            "tk", "tuk", "tuk" },
	{   "Twi" ,              "",            "tw", "twi", "twi" },
	{   "Uighur" ,           "",            "ug", "uig", "uig" },
	{   "Ukrainian" ,        "укра?нська мова", "uk", "ukr", "ukr" },
	{   "Urdu" ,             "",            "ur", "urd", "urd" },
	{   "Uzbek" ,            "",            "uz", "uzb", "uzb" },
	{   "Vietnamese" ,       "",            "vi", "vie", "vie" },
	{   "Volapuk" ,          "",            "vo", "vol", "vol" },
	{   "Welsh" ,            "",            "cy", "cym", "wel" },
	{   "Wolof" ,            "",            "wo", "wol", "wol" },
	{   "Xhosa" ,            "",            "xh", "xho", "xho" },
	{   "Yiddish" ,          "",            "yi", "yid", "yid" },
	{   "Yoruba" ,           "",            "yo", "yor", "yor" },
	{   "Zhuang" ,           "",            "za", "zha", "zha" },
	{   "Zulu" ,             "",            "zu", "zul", "zul" },
	{ NULL, NULL, NULL, NULL, NULL }
};
static const iso639_lang_t unknown_language ={ "Unknown", "Unknown", "??", "???", "???" };
const iso639_lang_t * GetLang_1( const char * psz_code );
const iso639_lang_t * GetLang_2T( const char * psz_code );
const iso639_lang_t * GetLang_2B( const char * psz_code );

char* file_get_line( char* s_str, int s_len, char* buf )
{
	char* ptr;
	if( ( s_str == NULL ) || ( s_len <= 0 ) )
        return NULL;

	while( ( ( *s_str == '\r' ) || ( *s_str =='\n' ) || ( *s_str == ' ' ) ) && ( s_len > 0 ) )
	{
		s_str++;
		s_len--;
	}

	if( s_len <= 0 )
        return NULL;

	ptr = s_str;

	while( s_len > 0 )
	{
		if( ( *s_str == '\r' ) || ( *s_str == '\n' ) )
			break;
		s_str++;
		s_len--;
	}
	if( ( s_str - ptr ) > 2047 )
		buf[0] = '\0';
	else
		strncpy( buf, ptr, s_str - ptr );
	buf[s_str - ptr] = '\0';
	return s_str;
}

int line_get_tag( char* str, int size, char* tag, char* value )
{
	char*	ptr;
	int		len;
	ptr = strchr( str, '=' );
	if( ptr == NULL )
		return -1;
	len = ptr - str;
	if( GetFrontValue( ptr - 1, len, tag ) < 0 )
		return -1;
	return GetRemainValue( ptr + 1, size + str - ptr - 1, value );
}

int line_get_bracket( char* s_str, int s_len, char* title )
{
	char *ptr;

	if( ( s_str == NULL ) || ( s_len <= 0 ) )
		return -1;

	while( ( *s_str != '[' ) && ( s_len > 0 ) )
	{
		s_str++;
		s_len--;
	}
	s_str++;
	s_len--;
	if( s_len <= 0 )
		return -1;

	while( ( *s_str == ' ' ) && ( s_len > 0 ) )
	{
		s_str++;
		s_len--;
	}
	if( s_len <= 0 )
		return -1;

	ptr = s_str;

	while( ( ( *s_str != ' ' ) && ( *s_str != ']' ) ) && ( s_len > 0 ) )
	{
		s_str++;
		s_len--;
	}
	if( ( s_str - ptr ) > 1023 )
		return -1;
	strncpy( title, ptr, s_str - ptr );
	title[s_str-ptr] = '\0';
	return 0;
}

int line_to_cap( char* str, int size )
{
	while( size )
	{
		if( ( *str >= 'a' ) && ( *str <= 'z' ) )
		{
			*str += 'A' - 'a';
		}
		str++;
		size--;
	}
	return 0;
}

int GetFrontValue( char* ptr, int size, char* value )
{
	char*	p;
	while( size )
	{
		if( ( *ptr != ' ' ) && ( *ptr != '\r' ) && ( *ptr != '\n' ) && ( *ptr != '\"' ) )
			break;
		ptr--;
		size--;
	}
	p = ptr;
	while( size )
	{
		/*lh  fixed channel name "CCTV HD"  */
		//if( ( *ptr == ' ' ) || ( *ptr == '\r' ) || ( *ptr == '\n' ) || ( *ptr == '\"' ) )  
		if( ( *ptr == '\r' ) || ( *ptr == '\n' ) || ( *ptr == '\"' ) )
			break;
		ptr--;
		size--;
	}
	strncpy( value, ptr + 1, p - ptr );
	value[p-ptr] = '\0';
	return 0;
}

int GetRemainValue( char* ptr, int size, char* value )
{
	char*	p;
	while( size )
	{
		if( ( *ptr != ' ' ) && ( *ptr != '\r' ) && ( *ptr != '\n' ) && ( *ptr != '\"' ) )
			break;
		ptr++;
		size--;
	}
	if( size == 0 )
		return -1;
	p = ptr;
	while( size )
	{
		if( ( *( ptr + size ) != ' ' ) && ( *( ptr + size ) != '\r' ) && ( *( ptr + size ) != '\n' ) )
			break;
		size--;
	}
	if( size == 0 )
		return -1;
	strncpy( value, p, size );
	value[size] = '\0';
	return 0;
}

int GetAfterValue( char* ptr, int size, char* value )
{
	char*	p;
	while( size )
	{
		if( ( *ptr != ' ' ) && ( *ptr != '\r' ) && ( *ptr != '\n' ) && ( *ptr != '\"' ) )
			break;
		ptr++;
		size--;
	}
	if( size == 0 )
		return -1;
	p = ptr;
	while( size )
	{
		if( ( *ptr == ' ' ) || ( *ptr == '\r' ) || ( *ptr == '\n' ) || ( *ptr == '\"' ) )
			break;
		ptr++;
		size--;
	}
	strncpy( value, p, ptr-p );
	value[ptr-p] = '\0';
	return 0;
}

const iso639_lang_t * GetLang_1( const char * psz_code )
{
	const iso639_lang_t *p_lang;

	for( p_lang = p_languages; p_lang->psz_eng_name; p_lang++ )
		if( !strncasecmp( p_lang->psz_iso639_1, psz_code, 2 ) )
			return p_lang;

	return &unknown_language;
}

const iso639_lang_t * GetLang_2T( const char * psz_code )
{
	const iso639_lang_t *p_lang;

	for( p_lang = p_languages; p_lang->psz_eng_name; p_lang++ )
		if( !strncasecmp( p_lang->psz_iso639_2T, psz_code, 3 ) )
			return p_lang;

	return &unknown_language;
}

const iso639_lang_t * GetLang_2B( const char * psz_code )
{
	const iso639_lang_t *p_lang;

	for( p_lang = p_languages; p_lang->psz_eng_name; p_lang++ )
		if( !strncasecmp( p_lang->psz_iso639_2B, psz_code, 3 ) )
			return p_lang;

	return &unknown_language;
}
/**************************************************************************
 * mid_get_language_full_name_from_iso639()
 *
 *  Description:
 *      find out the full name by ISO639 name
 *
 *  Input parameters:
 *      pIsoLanguage – the string of ISO639 name,
 *      pFullNameBuf - the buffer will used to store the full name  .
 *			fullNameBufLan - the length of the pFullNameBuf.
 *
 *  Return value:
 *      0 on success, -1 on failure
 *  Remark:
 *      
 **************************************************************************/
int mid_get_language_full_name_from_iso639(const char *pIsoLanguage, char *pFullNameBuf, int fullNameBufLan )
{
	const iso639_lang_t *pl = NULL;

	if( strlen( pIsoLanguage ) == 2 )
	{
		pl = GetLang_1( pIsoLanguage );
	}
	else if( strlen( pIsoLanguage ) == 3 )
	{
		pl = GetLang_2B( pIsoLanguage );
		if( !strcmp( pl->psz_iso639_1, "??" ) )
		{
			pl = GetLang_2T( pIsoLanguage );
		}
	}

	if( pl && strcmp( pl->psz_iso639_1, "??" ) )
	{
		strncpy(pFullNameBuf,pl->psz_eng_name,32);
	}

	return 0;
}


int	string_is_digital(const char* str)
{
	int			i = 0;
	int			len;

	if(NULL == str)
		return -1;

	i  = 0;
	len = strlen(str);
	while(i++ < len)
	{
		if(!isdigit(str[i]))
			break;
	}
	if(i < len)
		return 0;
	else
		return 1;
}

