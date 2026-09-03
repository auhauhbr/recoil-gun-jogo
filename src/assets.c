#include "assets.h"

#include <string.h>

static Texture2D carregar_textura(const char *caminho)
{
    Texture2D textura = LoadTexture(caminho);

    if (IsTextureValid(textura))
    {
        SetTextureFilter(textura, TEXTURE_FILTER_BILINEAR);
    }

    return textura;
}

static Sound carregar_som(const char *caminho, float volume)
{
    Sound som = LoadSound(caminho);

    if (IsSoundValid(som))
    {
        SetSoundVolume(som, volume);
    }

    return som;
}

Assets assets_carregar(void)
{
    Assets assets = {0};

    assets.armas[ARMA_PISTOLA] = carregar_textura("assets/armas/pistola.png");
    assets.armas[ARMA_REVOLVER] = carregar_textura("assets/armas/revolver.png");
    assets.armas[ARMA_ESPINGARDA] = carregar_textura("assets/armas/espingarda.png");

    assets.balas[ARMA_PISTOLA] = carregar_textura("assets/projeteis/bala_pistola.png");
    assets.balas[ARMA_REVOLVER] = carregar_textura("assets/projeteis/bala_revolver.png");
    assets.balas[ARMA_ESPINGARDA] = carregar_textura("assets/projeteis/bala_espingarda.png");

    assets.muzzle = carregar_textura("assets/efeitos/muzzle.png");
    assets.faisca = carregar_textura("assets/efeitos/faisca.png");
    assets.fumaca = carregar_textura("assets/efeitos/fumaca.png");
    assets.brilho_powerup = carregar_textura("assets/efeitos/powerup.png");

    if (IsAudioDeviceReady())
    {
        assets.audio_pronto = true;
        assets.tiros[ARMA_PISTOLA] = carregar_som("assets/audio/tiro_pistola.wav", 0.34f);
        assets.tiros[ARMA_REVOLVER] = carregar_som("assets/audio/tiro_revolver.wav", 0.28f);
        assets.tiros[ARMA_ESPINGARDA] = carregar_som("assets/audio/tiro_espingarda.wav", 0.34f);
        assets.ricochete = carregar_som("assets/audio/ricochete.wav", 0.18f);
        assets.powerup = carregar_som("assets/audio/powerup.ogg", 0.42f);
        assets.impacto = carregar_som("assets/audio/impacto.ogg", 0.26f);
        assets.clique = carregar_som("assets/audio/clique.ogg", 0.34f);

        assets.musica = LoadMusicStream("assets/musica/tema.mp3");
        assets.musica_pronta = IsMusicValid(assets.musica);

        if (assets.musica_pronta)
        {
            SetMusicVolume(assets.musica, 0.18f);
            assets.musica.looping = true;
            PlayMusicStream(assets.musica);
        }
    }

    return assets;
}

void assets_descarregar(Assets *assets)
{
    for (int i = 0; i < TOTAL_ARMAS; i++)
    {
        if (IsTextureValid(assets->armas[i]))
        {
            UnloadTexture(assets->armas[i]);
        }

        if (IsTextureValid(assets->balas[i]))
        {
            UnloadTexture(assets->balas[i]);
        }
    }

    Texture2D *efeitos[] = {
        &assets->muzzle,
        &assets->faisca,
        &assets->fumaca,
        &assets->brilho_powerup
    };

    for (int i = 0; i < 4; i++)
    {
        if (IsTextureValid(*efeitos[i]))
        {
            UnloadTexture(*efeitos[i]);
        }
    }

    if (assets->audio_pronto)
    {
        for (int i = 0; i < TOTAL_ARMAS; i++)
        {
            if (IsSoundValid(assets->tiros[i]))
            {
                UnloadSound(assets->tiros[i]);
            }
        }

        Sound *sons[] = {
            &assets->ricochete,
            &assets->powerup,
            &assets->impacto,
            &assets->clique
        };

        for (int i = 0; i < 4; i++)
        {
            if (IsSoundValid(*sons[i]))
            {
                UnloadSound(*sons[i]);
            }
        }

        if (assets->musica_pronta)
        {
            StopMusicStream(assets->musica);
            UnloadMusicStream(assets->musica);
        }
    }

    memset(assets, 0, sizeof(*assets));
}

Texture2D assets_textura_arma(const Assets *assets, TipoArma tipo)
{
    return assets->armas[tipo];
}

Texture2D assets_textura_bala(const Assets *assets, TipoArma tipo)
{
    return assets->balas[tipo];
}

Sound assets_som_tiro(const Assets *assets, TipoArma tipo)
{
    return assets->tiros[tipo];
}
