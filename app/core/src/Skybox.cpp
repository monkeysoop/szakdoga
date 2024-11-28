#include "Skybox.hpp"

#include <filesystem>
#include <fstream>
#include <SDL2/SDL_image.h>

static void invert_image_RGBA(int pitchInPixels, int height, Uint32* image_pixels)
{
	int height_div_2 = height / 2;
	Uint32* lower_data  =image_pixels;
	Uint32* higher_data =image_pixels + ( height - 1 ) * pitchInPixels;

	for ( int index = 0; index < height_div_2; index++ )
	{
		for ( int rowIndex = 0; rowIndex < pitchInPixels; rowIndex++ )
		{
			*lower_data ^= higher_data[ rowIndex ];
			higher_data[ rowIndex ] ^= *lower_data;
			*lower_data ^= higher_data[ rowIndex ];

			lower_data++;
		}
		higher_data -= pitchInPixels;
	}
}

void TextureFromFile( const GLuint tex, const std::filesystem::path& fileName, GLenum Type, GLenum Role )
{
	if ( tex == 0 )
	{
		SDL_LogMessage( SDL_LOG_CATEGORY_ERROR, 
						SDL_LOG_PRIORITY_ERROR,
						"Texture object needs to be inited before loading %s !", fileName.string().c_str());
		return;
	}

	// Kép betöltése
	SDL_Surface* loaded_img = IMG_Load(fileName.string().c_str());

	if (loaded_img == nullptr)
	{
		SDL_LogMessage( SDL_LOG_CATEGORY_ERROR, 
						SDL_LOG_PRIORITY_ERROR,
						"[TextureFromFile] Error while loading texture: %s", fileName.string().c_str());
		return;
	}

	// Uint32-ben tárolja az SDL a színeket, ezért számít a bájtsorrend
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	Uint32 format = SDL_PIXELFORMAT_ABGR8888;
#else
	Uint32 format = SDL_PIXELFORMAT_RGBA8888;
#endif

	// Átalakítás 32bit RGBA formátumra, ha nem abban volt
	SDL_Surface* formattedSurf = SDL_ConvertSurfaceFormat(loaded_img, format, 0);
	SDL_FreeSurface(loaded_img);
	if (formattedSurf == nullptr)
	{
		SDL_LogMessage( SDL_LOG_CATEGORY_ERROR, 
						SDL_LOG_PRIORITY_ERROR,
						"[TextureFromFile] Error while processing texture");
		return;
	}

	// Áttérés SDL koordinátarendszerről ( (0,0) balfent ) OpenGL textúra-koordinátarendszerre ( (0,0) ballent )
	if ( Type != GL_TEXTURE_CUBE_MAP && Type != GL_TEXTURE_CUBE_MAP_ARRAY )
		invert_image_RGBA( formattedSurf->pitch / sizeof( Uint32 ), formattedSurf->h, reinterpret_cast<Uint32*>( formattedSurf->pixels ) );

	glBindTexture(Type, tex);
	glTexImage2D(
		Role, 						// melyik binding point-on van a textúra erőforrás, amihez tárolást rendelünk
		0, 							// melyik részletességi szint adatait határozzuk meg
		GL_RGBA, 					// textúra belső tárolási formátuma (GPU-n)
		formattedSurf->w, 			// szélesség
		formattedSurf->h, 			// magasság
		0, 							// nulla kell, hogy legyen ( https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml )
		GL_RGBA, 					// forrás (=CPU-n) formátuma
		GL_UNSIGNED_BYTE, 			// forrás egy pixelének egy csatornáját hogyan tároljuk
		formattedSurf->pixels);		// forráshoz pointer

	glBindTexture(Type, 0);

	// Használt SDL_Surface-k felszabadítása
	SDL_FreeSurface(formattedSurf);
}

void SetupTextureSampling( GLenum Target, GLuint textureID, bool generateMipMap )
{
	// mintavételezés beállításai
	glBindTexture( Target, textureID );
	if ( generateMipMap ) glGenerateMipmap( Target ); // Mipmap generálása
	glTexParameteri( Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); // bilineáris szürés nagyításkor (ez az alapértelmezett)
	glTexParameteri( Target, GL_TEXTURE_MIN_FILTER, generateMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR ); // trilineáris szűrés a mipmap-ekböl kicsinyítéskor
	// mi legyen az eredmény, ha a textúrán kívülröl próbálunk mintát venni?
	glTexParameteri( Target, GL_TEXTURE_WRAP_S, GL_REPEAT ); // vízszintesen
	glTexParameteri( Target, GL_TEXTURE_WRAP_T, GL_REPEAT ); // függölegesen
	glBindTexture( Target, 0 );
}


Skybox::Skybox() {}
Skybox::~Skybox() {}


void Skybox::InitTexture() {
	glGenTextures(1, &skyboxTextureID);
	TextureFromFile(skyboxTextureID, "assets/skybox_xpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	TextureFromFile(skyboxTextureID, "assets/skybox_xneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	TextureFromFile(skyboxTextureID, "assets/skybox_ypos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	TextureFromFile(skyboxTextureID, "assets/skybox_yneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	TextureFromFile(skyboxTextureID, "assets/skybox_zpos.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	TextureFromFile(skyboxTextureID, "assets/skybox_zneg.png", GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	SetupTextureSampling(GL_TEXTURE_CUBE_MAP, skyboxTextureID, false);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Skybox::CleanTexture() {
	glDeleteTextures(1, &skyboxTextureID);
}
