#include <raylib.h>

#include "assets.h"
#include "config.h"
#include "jogo.h"

static Rectangle calcular_destino_canvas(void)
{
    float escala_x = (float)GetScreenWidth() / LARGURA_LOGICA;
    float escala_y = (float)GetScreenHeight() / ALTURA_LOGICA;
    float escala = escala_x < escala_y ? escala_x : escala_y;
    float largura = LARGURA_LOGICA * escala;
    float altura = ALTURA_LOGICA * escala;

    return (Rectangle){
        ((float)GetScreenWidth() - largura) * 0.5f,
        ((float)GetScreenHeight() - altura) * 0.5f,
        largura,
        altura
    };
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(LARGURA_LOGICA, ALTURA_LOGICA, "Recoil Gun");
    SetWindowMinSize(360, 640);
    SetTargetFPS(60);

    InitAudioDevice();

    RenderTexture2D canvas = LoadRenderTexture(LARGURA_LOGICA, ALTURA_LOGICA);
    Assets assets = assets_carregar();
    Jogo jogo;
    jogo_inicializar(&jogo);

    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        if (delta > 0.05f)
        {
            delta = 0.05f;
        }

        jogo_atualizar(&jogo, &assets, delta);

        BeginTextureMode(canvas);
        jogo_desenhar(&jogo, &assets);
        EndTextureMode();

        BeginDrawing();
        ClearBackground((Color){7, 8, 12, 255});

        Rectangle destino = calcular_destino_canvas();
        Rectangle origem = {
            0.0f,
            0.0f,
            (float)LARGURA_LOGICA,
            -(float)ALTURA_LOGICA
        };

        DrawTexturePro(
            canvas.texture,
            origem,
            destino,
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE
        );

        EndDrawing();
    }

    jogo_finalizar(&jogo);
    assets_descarregar(&assets);
    UnloadRenderTexture(canvas);

    if (IsAudioDeviceReady())
    {
        CloseAudioDevice();
    }

    CloseWindow();
    return 0;
}
