#include "jogo.h"

#include "assets.h"
#include "fases.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define CAT_CENARIO UINT64_C(0x0001)
#define CAT_ARMA_JOGADOR UINT64_C(0x0002)
#define CAT_ARMA_INIMIGO UINT64_C(0x0004)
#define CAT_PROJETIL_JOGADOR UINT64_C(0x0008)
#define CAT_PROJETIL_INIMIGO UINT64_C(0x0010)

#define RAIO_PROJETIL_METROS 0.11f
#define TEMPO_FLASH 0.075f

static const ConfigArma CONFIG_ARMAS[TOTAL_ARMAS] = {
    [ARMA_PISTOLA] = {
        .velocidade = 14.0f,
        .recuo = 3.2f,
        .impulso_angular = 0.10f,
        .cooldown = 0.24f,
        .tempo_vida = 4.0f,
        .ricochetes = 2,
        .quantidade_projeteis = 1,
        .abertura_graus = 0.0f,
        .largura_visual = 88.0f,
        .altura_visual = 70.0f,
        .largura_colisor = 80.0f,
        .altura_colisor = 24.0f,
        .salto_disparo = 3.6f,
        .salto_chao = 5.8f,
        .cano_offset_x = 41.0f,
        .cano_offset_y = -15.0f,
        .nome = "PISTOLA"
    },
    [ARMA_REVOLVER] = {
        .velocidade = 17.5f,
        .recuo = 4.7f,
        .impulso_angular = 0.14f,
        .cooldown = 0.46f,
        .tempo_vida = 4.5f,
        .ricochetes = 3,
        .quantidade_projeteis = 1,
        .abertura_graus = 0.0f,
        .largura_visual = 94.0f,
        .altura_visual = 64.0f,
        .largura_colisor = 84.0f,
        .altura_colisor = 24.0f,
        .salto_disparo = 3.9f,
        .salto_chao = 6.2f,
        .cano_offset_x = 44.0f,
        .cano_offset_y = -13.0f,
        .nome = "REVÓLVER"
    },
    [ARMA_ESPINGARDA] = {
        .velocidade = 12.8f,
        .recuo = 6.2f,
        .impulso_angular = 0.17f,
        .cooldown = 0.72f,
        .tempo_vida = 2.7f,
        .ricochetes = 1,
        .quantidade_projeteis = 5,
        .abertura_graus = 8.0f,
        .largura_visual = 132.0f,
        .altura_visual = 38.0f,
        .largura_colisor = 96.0f,
        .altura_colisor = 24.0f,
        .salto_disparo = 4.4f,
        .salto_chao = 7.0f,
        .cano_offset_x = 64.0f,
        .cano_offset_y = -7.0f,
        .nome = "ESPINGARDA"
    }
};

static float px_para_m(float pixels)
{
    return pixels / PIXELS_POR_METRO;
}

static float m_para_px(float metros)
{
    return metros * PIXELS_POR_METRO;
}

static float limitar(float valor, float minimo, float maximo)
{
    if (valor < minimo)
    {
        return minimo;
    }

    if (valor > maximo)
    {
        return maximo;
    }

    return valor;
}

static b2Vec2 vetor_rotacionado(b2Vec2 vetor, float angulo)
{
    float c = cosf(angulo);
    float s = sinf(angulo);

    return (b2Vec2){
        vetor.x * c - vetor.y * s,
        vetor.x * s + vetor.y * c
    };
}

static float comprimento_b2(b2Vec2 vetor)
{
    return sqrtf(vetor.x * vetor.x + vetor.y * vetor.y);
}

static b2Vec2 normalizar_b2(b2Vec2 vetor)
{
    float comprimento = comprimento_b2(vetor);

    if (comprimento < 0.0001f)
    {
        return (b2Vec2){1.0f, 0.0f};
    }

    return (b2Vec2){vetor.x / comprimento, vetor.y / comprimento};
}

static Rectangle botao_arma(int indice)
{
    return (Rectangle){342.0f + indice * 62.0f, 52.0f, 56.0f, 30.0f};
}

static Rectangle botao_jogar(void)
{
    return (Rectangle){155.0f, 680.0f, 230.0f, 56.0f};
}

static Rectangle botao_sobrevivencia(void)
{
    return (Rectangle){155.0f, 748.0f, 230.0f, 56.0f};
}

static Rectangle botao_continuar(void)
{
    return (Rectangle){145.0f, 655.0f, 250.0f, 64.0f};
}

static Vector2 posicao_entrada_logica(void)
{
    int largura = GetScreenWidth();
    int altura = GetScreenHeight();
    float escala_x = (float)largura / LARGURA_LOGICA;
    float escala_y = (float)altura / ALTURA_LOGICA;
    float escala = fminf(escala_x, escala_y);

    if (escala <= 0.0f)
    {
        return (Vector2){-1000.0f, -1000.0f};
    }

    float largura_destino = LARGURA_LOGICA * escala;
    float altura_destino = ALTURA_LOGICA * escala;
    float offset_x = ((float)largura - largura_destino) * 0.5f;
    float offset_y = ((float)altura - altura_destino) * 0.5f;

    Vector2 tela = GetMousePosition();

    if (GetTouchPointCount() > 0)
    {
        tela = GetTouchPosition(0);
    }

    return (Vector2){
        (tela.x - offset_x) / escala,
        (tela.y - offset_y) / escala
    };
}

static bool entrada_nova_pressao(Jogo *jogo)
{
    bool toque_atual = GetTouchPointCount() > 0;
    bool toque_novo = toque_atual && !jogo->toque_anterior;
    jogo->toque_anterior = toque_atual;

    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || toque_novo;
}

static int contar_projeteis(const Jogo *jogo, int filtro_dono)
{
    int quantidade = 0;

    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        if (!jogo->projeteis[i].ativo)
        {
            continue;
        }

        if (filtro_dono >= 0 && jogo->projeteis[i].dono != (Dono)filtro_dono)
        {
            continue;
        }

        quantidade++;
    }

    return quantidade;
}

static MarcadorColisao *novo_marcador_cenario(Jogo *jogo)
{
    if (jogo->quantidade_marcadores_cenario >= MAX_MARCADORES_CENARIO)
    {
        return NULL;
    }

    MarcadorColisao *marcador =
        &jogo->marcadores_cenario[jogo->quantidade_marcadores_cenario++];

    *marcador = (MarcadorColisao){
        .tipo = OBJETO_CENARIO,
        .dono = DONO_JOGADOR,
        .indice = -1
    };

    return marcador;
}

static b2BodyId criar_caixa_estatica(Jogo *jogo, Rectangle retangulo)
{
    b2BodyDef corpo_def = b2DefaultBodyDef();
    corpo_def.position = (b2Vec2){
        px_para_m(retangulo.x + retangulo.width * 0.5f),
        px_para_m(retangulo.y + retangulo.height * 0.5f)
    };

    b2BodyId corpo = b2CreateBody(jogo->mundo, &corpo_def);
    b2Polygon caixa = b2MakeBox(
        px_para_m(retangulo.width * 0.5f),
        px_para_m(retangulo.height * 0.5f)
    );

    b2ShapeDef forma_def = b2DefaultShapeDef();
    forma_def.userData = novo_marcador_cenario(jogo);
    forma_def.filter.categoryBits = CAT_CENARIO;
    forma_def.filter.maskBits =
        CAT_ARMA_JOGADOR |
        CAT_ARMA_INIMIGO |
        CAT_PROJETIL_JOGADOR |
        CAT_PROJETIL_INIMIGO;
    forma_def.enableContactEvents = true;
    // Eu mantenho o cenário pouco aderente e levemente elástico para evitar armas "coladas".
    forma_def.material.friction = 0.30f;
    forma_def.material.restitution = 0.28f;

    b2CreatePolygonShape(corpo, &forma_def, &caixa);
    return corpo;
}

static void criar_arma_fisica(
    Jogo *jogo,
    Arma *arma,
    Dono dono,
    TipoArma tipo,
    Vector2 posicao,
    float angulo_graus,
    int vida
)
{
    const ConfigArma *config = &CONFIG_ARMAS[tipo];

    *arma = (Arma){0};
    arma->dono = dono;
    arma->tipo = tipo;
    arma->vida = vida;
    arma->vida_maxima = vida;
    arma->marcador = (MarcadorColisao){
        .tipo = OBJETO_ARMA,
        .dono = dono,
        .indice = -1
    };

    b2BodyDef corpo_def = b2DefaultBodyDef();
    corpo_def.type = b2_dynamicBody;
    corpo_def.position = (b2Vec2){px_para_m(posicao.x), px_para_m(posicao.y)};
    corpo_def.rotation = b2MakeRot(angulo_graus * GRAUS_PARA_RAD);
    // Eu deixo a arma solta o bastante para tombar e responder aos disparos.
    corpo_def.linearDamping = 0.07f;
    corpo_def.angularDamping = 0.10f;

    arma->corpo = b2CreateBody(jogo->mundo, &corpo_def);

    b2Polygon caixa = b2MakeBox(
        px_para_m(config->largura_colisor * 0.5f),
        px_para_m(config->altura_colisor * 0.5f)
    );

    b2ShapeDef forma_def = b2DefaultShapeDef();
    forma_def.userData = &arma->marcador;
    forma_def.density = 1.0f;
    // Pouco atrito + quique moderado deixam o chão recuperável, não um ponto morto.
    forma_def.material.friction = 0.26f;
    forma_def.material.restitution = 0.42f;
    forma_def.enableContactEvents = true;

    if (dono == DONO_JOGADOR)
    {
        forma_def.filter.categoryBits = CAT_ARMA_JOGADOR;
        forma_def.filter.maskBits = CAT_CENARIO | CAT_PROJETIL_INIMIGO;
    }
    else
    {
        forma_def.filter.categoryBits = CAT_ARMA_INIMIGO;
        forma_def.filter.maskBits = CAT_CENARIO | CAT_PROJETIL_JOGADOR;
    }

    arma->forma = b2CreatePolygonShape(arma->corpo, &forma_def, &caixa);
}

static int encontrar_projetil_livre(Jogo *jogo)
{
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        if (!jogo->projeteis[i].ativo)
        {
            return i;
        }
    }

    return -1;
}

static Projetil *criar_projetil_em(
    Jogo *jogo,
    Dono dono,
    TipoArma tipo_arma,
    b2Vec2 posicao,
    b2Vec2 velocidade,
    float tempo_restante,
    int ricochetes
)
{
    int indice = encontrar_projetil_livre(jogo);

    if (indice < 0)
    {
        return NULL;
    }

    Projetil *projetil = &jogo->projeteis[indice];
    *projetil = (Projetil){0};
    projetil->ativo = true;
    projetil->dono = dono;
    projetil->tipo_arma = tipo_arma;
    projetil->tempo_restante = tempo_restante;
    projetil->ricochetes_restantes = ricochetes;
    projetil->marcador = (MarcadorColisao){
        .tipo = OBJETO_PROJETIL,
        .dono = dono,
        .indice = indice
    };

    b2BodyDef corpo_def = b2DefaultBodyDef();
    corpo_def.type = b2_dynamicBody;
    corpo_def.position = posicao;
    corpo_def.gravityScale = 0.0f;
    corpo_def.isBullet = true;

    projetil->corpo = b2CreateBody(jogo->mundo, &corpo_def);

    b2Circle circulo = {
        .center = {0.0f, 0.0f},
        .radius = RAIO_PROJETIL_METROS
    };

    b2ShapeDef forma_def = b2DefaultShapeDef();
    forma_def.userData = &projetil->marcador;
    forma_def.density = 0.16f;
    forma_def.material.friction = 0.0f;
    forma_def.material.restitution = 1.0f;
    forma_def.enableContactEvents = true;

    if (dono == DONO_JOGADOR)
    {
        forma_def.filter.categoryBits = CAT_PROJETIL_JOGADOR;
        forma_def.filter.maskBits = CAT_CENARIO | CAT_ARMA_INIMIGO;
    }
    else
    {
        forma_def.filter.categoryBits = CAT_PROJETIL_INIMIGO;
        forma_def.filter.maskBits = CAT_CENARIO | CAT_ARMA_JOGADOR;
    }

    projetil->forma = b2CreateCircleShape(projetil->corpo, &forma_def, &circulo);
    b2Body_SetLinearVelocity(projetil->corpo, velocidade);

    return projetil;
}

static void emitir_particulas(
    Jogo *jogo,
    Vector2 posicao,
    Color cor,
    int quantidade,
    float velocidade
)
{
    for (int n = 0; n < quantidade; n++)
    {
        int indice = -1;

        for (int i = 0; i < MAX_PARTICULAS; i++)
        {
            if (!jogo->particulas[i].ativo)
            {
                indice = i;
                break;
            }
        }

        if (indice < 0)
        {
            return;
        }

        float angulo = GetRandomValue(0, 359) * GRAUS_PARA_RAD;
        float fator = GetRandomValue(45, 100) / 100.0f;
        float vida = GetRandomValue(25, 55) / 100.0f;

        jogo->particulas[indice] = (Particula){
            .ativo = true,
            .posicao = posicao,
            .velocidade = {
                cosf(angulo) * velocidade * fator,
                sinf(angulo) * velocidade * fator
            },
            .vida = vida,
            .vida_total = vida,
            .tamanho = (float)GetRandomValue(3, 8),
            .cor = cor
        };
    }
}

static b2Vec2 posicao_cano_fisica(const Arma *arma)
{
    const ConfigArma *config = &CONFIG_ARMAS[arma->tipo];
    b2Vec2 centro = b2Body_GetPosition(arma->corpo);
    float angulo = b2Rot_GetAngle(b2Body_GetRotation(arma->corpo));

    float x_local = px_para_m(config->cano_offset_x);
    float y_local = px_para_m(config->cano_offset_y);

    /*
     * Eu trato a ponta do cano como um ponto local do sprite.
     * Assim ela acompanha posição E rotação da arma em todos os frames.
     */
    return (b2Vec2){
        centro.x + cosf(angulo) * x_local - sinf(angulo) * y_local,
        centro.y + sinf(angulo) * x_local + cosf(angulo) * y_local
    };
}

static Vector2 posicao_cano(const Arma *arma)
{
    b2Vec2 cano = posicao_cano_fisica(arma);
    return (Vector2){m_para_px(cano.x), m_para_px(cano.y)};
}

static void tocar_som_seguro(bool sem_audio, Sound som)
{
    if (!sem_audio && IsSoundValid(som))
    {
        PlaySound(som);
    }
}

static bool arma_disparar(Jogo *jogo, Arma *arma, Assets *assets)
{
    if (arma->cooldown_restante > 0.0f || arma->vida <= 0)
    {
        return false;
    }

    const ConfigArma *config = &CONFIG_ARMAS[arma->tipo];
    b2Vec2 posicao_arma = b2Body_GetPosition(arma->corpo);
    float angulo = b2Rot_GetAngle(b2Body_GetRotation(arma->corpo));
    b2Vec2 direcao_base = {cosf(angulo), sinf(angulo)};
    b2Vec2 origem = posicao_cano_fisica(arma);

    int quantidade = config->quantidade_projeteis;

    for (int i = 0; i < quantidade; i++)
    {
        float deslocamento = i - (quantidade - 1) * 0.5f;
        float angulo_abertura = deslocamento * config->abertura_graus * GRAUS_PARA_RAD;
        b2Vec2 direcao = vetor_rotacionado(direcao_base, angulo_abertura);
        b2Vec2 velocidade = {
            direcao.x * config->velocidade,
            direcao.y * config->velocidade
        };

        criar_projetil_em(
            jogo,
            arma->dono,
            arma->tipo,
            origem,
            velocidade,
            config->tempo_vida,
            config->ricochetes
        );
    }

    b2Vec2 impulso = {
        -direcao_base.x * config->recuo,
        -direcao_base.y * config->recuo
    };

    /*
     * O jogo usa "recuo jogável", não balística pura:
     * cada tiro sempre dá sustentação para cima. Perto do chão,
     * o impulso vertical é maior para a arma nunca ficar sem saída.
     */
    float posicao_y_px = m_para_px(posicao_arma.y);
    bool perto_do_chao = posicao_y_px > jogo->chao.y - 105.0f;
    float salto_minimo = perto_do_chao ? config->salto_chao : config->salto_disparo;

    if (impulso.y > -salto_minimo)
    {
        impulso.y = -salto_minimo;
    }

    if (perto_do_chao)
    {
        b2Vec2 velocidade_atual = b2Body_GetLinearVelocity(arma->corpo);

        // Eu removo boa parte da queda antes do "salto" de recuperação.
        if (velocidade_atual.y > 0.0f)
        {
            velocidade_atual.y *= 0.12f;
            b2Body_SetLinearVelocity(arma->corpo, velocidade_atual);
        }
    }

    b2Body_ApplyLinearImpulseToCenter(arma->corpo, impulso, true);

    float sinal_giro = arma->dono == DONO_JOGADOR ? 1.0f : -1.0f;
    b2Body_ApplyAngularImpulse(
        arma->corpo,
        config->impulso_angular * sinal_giro,
        true
    );

    arma->cooldown_restante = config->cooldown;
    arma->tempo_flash = TEMPO_FLASH;

    tocar_som_seguro(jogo->sem_audio, assets_som_tiro(assets, arma->tipo));
    emitir_particulas(jogo, posicao_cano(arma), (Color){255, 190, 80, 255}, 5, 70.0f);
    return true;
}

static void destruir_projeteis_marcados(Jogo *jogo)
{
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        Projetil *projetil = &jogo->projeteis[i];

        if (!projetil->ativo || !projetil->remover)
        {
            continue;
        }

        if (b2Body_IsValid(projetil->corpo))
        {
            b2DestroyBody(projetil->corpo);
        }

        *projetil = (Projetil){0};
    }
}

static void processar_colisao_projetil_cenario(
    Jogo *jogo,
    Assets *assets,
    MarcadorColisao *marcador_projetil
)
{
    if (marcador_projetil->indice < 0 || marcador_projetil->indice >= MAX_PROJETEIS)
    {
        return;
    }

    Projetil *projetil = &jogo->projeteis[marcador_projetil->indice];

    if (!projetil->ativo || projetil->remover)
    {
        return;
    }

    projetil->ricochetes_restantes--;

    b2Vec2 pos = b2Body_GetPosition(projetil->corpo);
    emitir_particulas(
        jogo,
        (Vector2){m_para_px(pos.x), m_para_px(pos.y)},
        (Color){255, 214, 85, 255},
        4,
        85.0f
    );

    if (GetRandomValue(0, 2) == 0)
    {
        tocar_som_seguro(jogo->sem_audio, assets->ricochete);
    }

    if (projetil->ricochetes_restantes < 0)
    {
        projetil->remover = true;
    }
}

static void processar_colisao_projetil_arma(
    Jogo *jogo,
    Assets *assets,
    MarcadorColisao *marcador_projetil,
    MarcadorColisao *marcador_arma
)
{
    if (marcador_projetil->indice < 0 || marcador_projetil->indice >= MAX_PROJETEIS)
    {
        return;
    }

    Projetil *projetil = &jogo->projeteis[marcador_projetil->indice];

    if (!projetil->ativo || projetil->remover || projetil->dono == marcador_arma->dono)
    {
        return;
    }

    Arma *alvo = marcador_arma->dono == DONO_JOGADOR ? &jogo->jogador : &jogo->inimigo;
    alvo->vida--;
    projetil->remover = true;

    b2Vec2 pos = b2Body_GetPosition(alvo->corpo);
    emitir_particulas(
        jogo,
        (Vector2){m_para_px(pos.x), m_para_px(pos.y)},
        marcador_arma->dono == DONO_JOGADOR ? RED : ORANGE,
        14,
        145.0f
    );

    tocar_som_seguro(jogo->sem_audio, assets->impacto);

    if (projetil->dono == DONO_JOGADOR)
    {
        jogo->pontuacao += 120;
    }
}

static void processar_eventos_contato(Jogo *jogo, Assets *assets)
{
    b2ContactEvents eventos = b2World_GetContactEvents(jogo->mundo);

    for (int i = 0; i < eventos.beginCount; i++)
    {
        b2ContactBeginTouchEvent *evento = &eventos.beginEvents[i];

        if (!b2Shape_IsValid(evento->shapeIdA) || !b2Shape_IsValid(evento->shapeIdB))
        {
            continue;
        }

        MarcadorColisao *a = b2Shape_GetUserData(evento->shapeIdA);
        MarcadorColisao *b = b2Shape_GetUserData(evento->shapeIdB);

        if (a == NULL || b == NULL)
        {
            continue;
        }

        if (a->tipo == OBJETO_PROJETIL && b->tipo == OBJETO_CENARIO)
        {
            processar_colisao_projetil_cenario(jogo, assets, a);
        }
        else if (b->tipo == OBJETO_PROJETIL && a->tipo == OBJETO_CENARIO)
        {
            processar_colisao_projetil_cenario(jogo, assets, b);
        }
        else if (a->tipo == OBJETO_PROJETIL && b->tipo == OBJETO_ARMA)
        {
            processar_colisao_projetil_arma(jogo, assets, a, b);
        }
        else if (b->tipo == OBJETO_PROJETIL && a->tipo == OBJETO_ARMA)
        {
            processar_colisao_projetil_arma(jogo, assets, b, a);
        }
    }
}

static void atualizar_projeteis(Jogo *jogo, float delta)
{
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        Projetil *projetil = &jogo->projeteis[i];

        if (!projetil->ativo)
        {
            continue;
        }

        projetil->tempo_restante -= delta;

        if (projetil->tempo_restante <= 0.0f)
        {
            projetil->remover = true;
        }
    }
}

static void atualizar_particulas(Jogo *jogo, float delta)
{
    for (int i = 0; i < MAX_PARTICULAS; i++)
    {
        Particula *p = &jogo->particulas[i];

        if (!p->ativo)
        {
            continue;
        }

        p->vida -= delta;

        if (p->vida <= 0.0f)
        {
            p->ativo = false;
            continue;
        }

        p->posicao.x += p->velocidade.x * delta;
        p->posicao.y += p->velocidade.y * delta;
        p->velocidade.x *= 0.96f;
        p->velocidade.y *= 0.96f;
    }
}

static void multiplicar_projeteis(Jogo *jogo, Dono dono, int fator)
{
    int originais[MAX_PROJETEIS];
    int quantidade = 0;

    // Eu fotografo os índices antes de clonar para impedir uma multiplicação recursiva no mesmo frame.
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        if (jogo->projeteis[i].ativo && jogo->projeteis[i].dono == dono)
        {
            originais[quantidade++] = i;
        }
    }

    for (int n = 0; n < quantidade; n++)
    {
        Projetil *original = &jogo->projeteis[originais[n]];

        if (!original->ativo || original->remover)
        {
            continue;
        }

        b2Vec2 posicao = b2Body_GetPosition(original->corpo);
        b2Vec2 velocidade = b2Body_GetLinearVelocity(original->corpo);

        if (fator >= 2)
        {
            float sinal = (n % 2 == 0) ? -1.0f : 1.0f;
            criar_projetil_em(
                jogo,
                dono,
                original->tipo_arma,
                posicao,
                vetor_rotacionado(velocidade, sinal * 12.0f * GRAUS_PARA_RAD),
                original->tempo_restante,
                original->ricochetes_restantes
            );
        }

        if (fator >= 3)
        {
            float sinal = (n % 2 == 0) ? 1.0f : -1.0f;
            criar_projetil_em(
                jogo,
                dono,
                original->tipo_arma,
                posicao,
                vetor_rotacionado(velocidade, sinal * 12.0f * GRAUS_PARA_RAD),
                original->tempo_restante,
                original->ricochetes_restantes
            );
        }
    }
}

static void adicionar_ricochetes(Jogo *jogo, Dono dono, int quantidade)
{
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        Projetil *projetil = &jogo->projeteis[i];

        if (projetil->ativo && projetil->dono == dono)
        {
            projetil->ricochetes_restantes += quantidade;
        }
    }
}

static void acelerar_projeteis(Jogo *jogo, Dono dono, float multiplicador)
{
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        Projetil *projetil = &jogo->projeteis[i];

        if (!projetil->ativo || projetil->dono != dono)
        {
            continue;
        }

        b2Vec2 velocidade = b2Body_GetLinearVelocity(projetil->corpo);
        velocidade.x *= multiplicador;
        velocidade.y *= multiplicador;
        b2Body_SetLinearVelocity(projetil->corpo, velocidade);
    }
}

static const char *nome_powerup(TipoPowerup tipo)
{
    switch (tipo)
    {
        case POWERUP_DUPLO:
            return "2X";
        case POWERUP_TRIPLO:
            return "3X";
        case POWERUP_RICOCHETE:
            return "+2R";
        case POWERUP_VELOCIDADE:
            return "FAST";
        case POWERUP_CAVEIRA:
            return "SKULL";
        default:
            return "?";
    }
}

static void ativar_powerup(Jogo *jogo, Assets *assets, Powerup *powerup, Dono dono)
{
    powerup->ativo = false;

    switch (powerup->tipo)
    {
        case POWERUP_DUPLO:
            multiplicar_projeteis(jogo, dono, 2);
            break;

        case POWERUP_TRIPLO:
            multiplicar_projeteis(jogo, dono, 3);
            break;

        case POWERUP_RICOCHETE:
            adicionar_ricochetes(jogo, dono, 2);
            break;

        case POWERUP_VELOCIDADE:
            acelerar_projeteis(jogo, dono, 1.35f);
            break;

        case POWERUP_CAVEIRA:
            multiplicar_projeteis(jogo, dono, 2);
            adicionar_ricochetes(jogo, dono, 1);
            acelerar_projeteis(jogo, dono, 1.16f);
            break;
    }

    Vector2 centro = {
        powerup->area.x + powerup->area.width * 0.5f,
        powerup->area.y + powerup->area.height * 0.5f
    };

    emitir_particulas(jogo, centro, (Color){202, 85, 255, 255}, 22, 185.0f);
    tocar_som_seguro(jogo->sem_audio, assets->powerup);

    if (dono == DONO_JOGADOR)
    {
        jogo->pontuacao += 80;
    }
}

static void verificar_powerups(Jogo *jogo, Assets *assets)
{
    float raio = m_para_px(RAIO_PROJETIL_METROS);

    for (int p = 0; p < jogo->quantidade_powerups; p++)
    {
        Powerup *powerup = &jogo->powerups[p];

        if (!powerup->ativo)
        {
            continue;
        }

        powerup->pulso += 0.07f;

        for (int i = 0; i < MAX_PROJETEIS; i++)
        {
            Projetil *projetil = &jogo->projeteis[i];

            if (!projetil->ativo || projetil->remover)
            {
                continue;
            }

            b2Vec2 pos = b2Body_GetPosition(projetil->corpo);
            Vector2 tela = {m_para_px(pos.x), m_para_px(pos.y)};

            if (CheckCollisionCircleRec(tela, raio, powerup->area))
            {
                ativar_powerup(jogo, assets, powerup, projetil->dono);
                break;
            }
        }
    }
}

static void atualizar_arma(Arma *arma, float delta)
{
    arma->cooldown_restante = fmaxf(0.0f, arma->cooldown_restante - delta);
    arma->tempo_flash = fmaxf(0.0f, arma->tempo_flash - delta);
}

static void estabilizar_arma(Arma *arma)
{
    if (arma->vida <= 0 || !b2Body_IsValid(arma->corpo))
    {
        return;
    }

    b2Vec2 velocidade = b2Body_GetLinearVelocity(arma->corpo);
    velocidade.x = limitar(velocidade.x, -9.0f, 9.0f);
    velocidade.y = limitar(velocidade.y, -9.5f, 10.5f);
    b2Body_SetLinearVelocity(arma->corpo, velocidade);

    float velocidade_angular = b2Body_GetAngularVelocity(arma->corpo);
    velocidade_angular = limitar(velocidade_angular, -8.0f, 8.0f);
    b2Body_SetAngularVelocity(arma->corpo, velocidade_angular);
}

static void atualizar_inimigo(Jogo *jogo, Assets *assets, float delta)
{
    Arma *inimigo = &jogo->inimigo;
    inimigo->tempo_ia += delta;

    if (inimigo->cooldown_restante > 0.0f || inimigo->vida <= 0)
    {
        return;
    }

    b2Vec2 pos_inimigo = b2Body_GetPosition(inimigo->corpo);
    b2Vec2 pos_jogador = b2Body_GetPosition(jogo->jogador.corpo);
    b2Vec2 para_jogador = normalizar_b2((b2Vec2){
        pos_jogador.x - pos_inimigo.x,
        pos_jogador.y - pos_inimigo.y
    });

    float angulo = b2Rot_GetAngle(b2Body_GetRotation(inimigo->corpo));
    b2Vec2 frente = {cosf(angulo), sinf(angulo)};
    float alinhamento = frente.x * para_jogador.x + frente.y * para_jogador.y;

    float alinhamento_minimo = 0.48f;
    float tempo_disparo_forcado = 2.15f;
    float fator_cooldown = 1.0f;

    if (jogo->modo == MODO_SOBREVIVENCIA)
    {
        float nivel = (float)(jogo->onda_sobrevivencia - 1);
        alinhamento_minimo = fmaxf(0.25f, 0.48f - nivel * 0.018f);
        tempo_disparo_forcado = fmaxf(0.85f, 2.15f - nivel * 0.11f);
        fator_cooldown = fmaxf(0.58f, 1.0f - nivel * 0.035f);
    }

    if (alinhamento > alinhamento_minimo || inimigo->tempo_ia > tempo_disparo_forcado)
    {
        if (arma_disparar(jogo, inimigo, assets))
        {
            inimigo->cooldown_restante *= fator_cooldown;
            inimigo->tempo_ia = 0.0f;
        }
    }
}

static void limpar_runtime_fase(Jogo *jogo)
{
    memset(jogo->projeteis, 0, sizeof(jogo->projeteis));
    memset(jogo->particulas, 0, sizeof(jogo->particulas));
    memset(jogo->obstaculos, 0, sizeof(jogo->obstaculos));
    memset(jogo->powerups, 0, sizeof(jogo->powerups));
    memset(jogo->marcadores_cenario, 0, sizeof(jogo->marcadores_cenario));
    jogo->quantidade_obstaculos = 0;
    jogo->quantidade_powerups = 0;
    jogo->quantidade_marcadores_cenario = 0;
}

static void preparar_mundo_base(Jogo *jogo)
{
    if (jogo->mundo_criado)
    {
        b2DestroyWorld(jogo->mundo);
        jogo->mundo_criado = false;
    }

    limpar_runtime_fase(jogo);
    jogo->tempo_estado = 0.0f;

    jogo->arena = (Rectangle){20.0f, 100.0f, 500.0f, 820.0f};
    jogo->parede_esquerda = (Rectangle){20.0f, 100.0f, 12.0f, 820.0f};
    jogo->parede_direita = (Rectangle){508.0f, 100.0f, 12.0f, 820.0f};
    jogo->teto = (Rectangle){20.0f, 100.0f, 500.0f, 12.0f};
    jogo->chao = (Rectangle){20.0f, 908.0f, 500.0f, 12.0f};

    b2WorldDef mundo_def = b2DefaultWorldDef();
    mundo_def.gravity = (b2Vec2){0.0f, 6.4f};
    jogo->mundo = b2CreateWorld(&mundo_def);
    jogo->mundo_criado = true;

    criar_caixa_estatica(jogo, jogo->parede_esquerda);
    criar_caixa_estatica(jogo, jogo->parede_direita);
    criar_caixa_estatica(jogo, jogo->teto);
    criar_caixa_estatica(jogo, jogo->chao);
}

static float angulo_para_alvo(Vector2 origem, Vector2 alvo)
{
    return atan2f(alvo.y - origem.y, alvo.x - origem.x) * RAD_PARA_GRAUS;
}

static void iniciar_fase(Jogo *jogo, int indice)
{
    preparar_mundo_base(jogo);
    jogo->fase_atual = indice;

    DefinicaoFase fase = fase_obter(indice);

    jogo->quantidade_obstaculos = fase.quantidade_obstaculos;
    for (int i = 0; i < fase.quantidade_obstaculos; i++)
    {
        jogo->obstaculos[i].area = fase.obstaculos[i];
        jogo->obstaculos[i].corpo = criar_caixa_estatica(jogo, fase.obstaculos[i]);
    }

    jogo->quantidade_powerups = fase.quantidade_powerups;
    for (int i = 0; i < fase.quantidade_powerups; i++)
    {
        jogo->powerups[i] = (Powerup){
            .ativo = true,
            .area = fase.areas_powerups[i],
            .tipo = fase.tipos_powerups[i],
            .pulso = i * 0.8f
        };
    }

    /*
     * Eu calculo a orientação inicial a partir da posição real dos dois corpos.
     * Desse modo as armas sempre começam olhando uma para a outra.
     */
    float angulo_jogador = angulo_para_alvo(fase.posicao_jogador, fase.posicao_inimigo);
    float angulo_inimigo = angulo_para_alvo(fase.posicao_inimigo, fase.posicao_jogador);

    criar_arma_fisica(
        jogo,
        &jogo->jogador,
        DONO_JOGADOR,
        jogo->arma_selecionada,
        fase.posicao_jogador,
        angulo_jogador,
        fase.vida_jogador
    );

    criar_arma_fisica(
        jogo,
        &jogo->inimigo,
        DONO_INIMIGO,
        fase.arma_inimigo,
        fase.posicao_inimigo,
        angulo_inimigo,
        fase.vida_inimigo
    );

    jogo->estado = ESTADO_JOGANDO;
}

static void adicionar_obstaculos_sobrevivencia(Jogo *jogo)
{
    static const Rectangle candidatos[] = {
        {70.0f, 430.0f, 135.0f, 14.0f},
        {335.0f, 430.0f, 135.0f, 14.0f},
        {195.0f, 565.0f, 150.0f, 14.0f},
        {70.0f, 700.0f, 135.0f, 14.0f},
        {335.0f, 700.0f, 135.0f, 14.0f},
        {195.0f, 335.0f, 150.0f, 14.0f}
    };

    int quantidade = 1 + (jogo->onda_sobrevivencia - 1) / 2;
    if (quantidade > 6)
    {
        quantidade = 6;
    }

    int inicio = (jogo->onda_sobrevivencia - 1) % 6;
    jogo->quantidade_obstaculos = quantidade;

    for (int i = 0; i < quantidade; i++)
    {
        Rectangle area = candidatos[(inicio + i) % 6];
        jogo->obstaculos[i].area = area;
        jogo->obstaculos[i].corpo = criar_caixa_estatica(jogo, area);
    }
}

static void adicionar_powerups_sobrevivencia(Jogo *jogo)
{
    static const Rectangle posicoes[] = {
        {52.0f, 790.0f, 54.0f, 54.0f},
        {434.0f, 790.0f, 54.0f, 54.0f},
        {243.0f, 620.0f, 54.0f, 54.0f}
    };

    int quantidade = 1 + (jogo->onda_sobrevivencia - 1) / 4;
    if (quantidade > 3)
    {
        quantidade = 3;
    }

    jogo->quantidade_powerups = quantidade;

    for (int i = 0; i < quantidade; i++)
    {
        int deslocamento = (jogo->onda_sobrevivencia + i) % 3;
        int indice_posicao = (i + deslocamento) % 3;
        TipoPowerup tipo = (TipoPowerup)GetRandomValue(POWERUP_DUPLO, POWERUP_CAVEIRA);

        jogo->powerups[i] = (Powerup){
            .ativo = true,
            .area = posicoes[indice_posicao],
            .tipo = tipo,
            .pulso = i * 0.8f
        };
    }
}

static void iniciar_onda_sobrevivencia(Jogo *jogo)
{
    preparar_mundo_base(jogo);

    Vector2 posicao_jogador = {145.0f, 275.0f};
    Vector2 posicao_inimigo = {395.0f, 275.0f};

    adicionar_obstaculos_sobrevivencia(jogo);
    adicionar_powerups_sobrevivencia(jogo);

    int vida_jogador = jogo->vida_sobrevivencia > 0 ? jogo->vida_sobrevivencia : 7;

    criar_arma_fisica(
        jogo,
        &jogo->jogador,
        DONO_JOGADOR,
        jogo->arma_selecionada,
        posicao_jogador,
        angulo_para_alvo(posicao_jogador, posicao_inimigo),
        vida_jogador
    );

    TipoArma arma_inimigo = (TipoArma)((jogo->onda_sobrevivencia - 1) % TOTAL_ARMAS);
    int vida_inimigo = 4 + (jogo->onda_sobrevivencia - 1) / 2;
    if (vida_inimigo > 14)
    {
        vida_inimigo = 14;
    }

    criar_arma_fisica(
        jogo,
        &jogo->inimigo,
        DONO_INIMIGO,
        arma_inimigo,
        posicao_inimigo,
        angulo_para_alvo(posicao_inimigo, posicao_jogador),
        vida_inimigo
    );

    jogo->estado = ESTADO_JOGANDO;
}

static void trocar_arma_jogador(Jogo *jogo, TipoArma tipo)
{
    jogo->arma_selecionada = tipo;
    jogo->jogador.tipo = tipo;
    jogo->jogador.cooldown_restante = fmaxf(jogo->jogador.cooldown_restante, 0.12f);
}

static void verificar_fim_fase(Jogo *jogo)
{
    if (jogo->inimigo.vida <= 0)
    {
        jogo->inimigo.vida = 0;
        jogo->pontuacao += 500 + jogo->fase_atual * 150;
        jogo->estado = ESTADO_VITORIA_FASE;
        jogo->tempo_estado = 0.0f;
    }
    else if (jogo->jogador.vida <= 0)
    {
        jogo->jogador.vida = 0;
        jogo->estado = ESTADO_DERROTA;
        jogo->tempo_estado = 0.0f;
    }
}

void jogo_inicializar(Jogo *jogo)
{
    memset(jogo, 0, sizeof(*jogo));
    jogo->estado = ESTADO_MENU;
    jogo->modo = MODO_CAMPANHA;
    jogo->arma_selecionada = ARMA_PISTOLA;
    jogo->fase_atual = 0;
}

void jogo_finalizar(Jogo *jogo)
{
    if (jogo->mundo_criado)
    {
        b2DestroyWorld(jogo->mundo);
        jogo->mundo_criado = false;
    }
}

static void atualizar_menu(Jogo *jogo, Assets *assets, bool clicou, Vector2 cursor)
{
    if (IsKeyPressed(KEY_ONE))
    {
        jogo->arma_selecionada = ARMA_PISTOLA;
    }
    else if (IsKeyPressed(KEY_TWO))
    {
        jogo->arma_selecionada = ARMA_REVOLVER;
    }
    else if (IsKeyPressed(KEY_THREE))
    {
        jogo->arma_selecionada = ARMA_ESPINGARDA;
    }

    if (clicou)
    {
        for (int i = 0; i < TOTAL_ARMAS; i++)
        {
            Rectangle carta = {70.0f + i * 140.0f, 465.0f, 120.0f, 150.0f};

            if (CheckCollisionPointRec(cursor, carta))
            {
                jogo->arma_selecionada = (TipoArma)i;
                tocar_som_seguro(jogo->sem_audio, assets->clique);
                return;
            }
        }
    }

    if (IsKeyPressed(KEY_ENTER) || (clicou && CheckCollisionPointRec(cursor, botao_jogar())))
    {
        tocar_som_seguro(jogo->sem_audio, assets->clique);
        jogo->modo = MODO_CAMPANHA;
        jogo->pontuacao = 0;
        iniciar_fase(jogo, 0);
        return;
    }

    if (IsKeyPressed(KEY_S) ||
        (clicou && CheckCollisionPointRec(cursor, botao_sobrevivencia())))
    {
        tocar_som_seguro(jogo->sem_audio, assets->clique);
        jogo->modo = MODO_SOBREVIVENCIA;
        jogo->pontuacao = 0;
        jogo->onda_sobrevivencia = 1;
        jogo->vida_sobrevivencia = 7;
        jogo->combo = 0;
        jogo->maior_combo = 0;
        jogo->tempo_combo = 0.0f;
        iniciar_onda_sobrevivencia(jogo);
    }
}

static void atualizar_estado_finalizado(Jogo *jogo, Assets *assets, bool clicou, Vector2 cursor)
{
    jogo->tempo_estado += GetFrameTime();

    if (jogo->estado == ESTADO_DERROTA)
    {
        if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER) ||
            (clicou && CheckCollisionPointRec(cursor, botao_continuar())))
        {
            tocar_som_seguro(jogo->sem_audio, assets->clique);
            iniciar_fase(jogo, jogo->fase_atual);
        }
        return;
    }

    if (jogo->estado == ESTADO_FINAL)
    {
        if (IsKeyPressed(KEY_ENTER) ||
            (clicou && CheckCollisionPointRec(cursor, botao_continuar())))
        {
            tocar_som_seguro(jogo->sem_audio, assets->clique);
            if (jogo->mundo_criado)
            {
                b2DestroyWorld(jogo->mundo);
                jogo->mundo_criado = false;
            }
            jogo->estado = ESTADO_MENU;
        }
        return;
    }

    if (IsKeyPressed(KEY_ENTER) ||
        (clicou && CheckCollisionPointRec(cursor, botao_continuar())))
    {
        tocar_som_seguro(jogo->sem_audio, assets->clique);

        if (jogo->fase_atual + 1 >= TOTAL_FASES)
        {
            jogo->estado = ESTADO_FINAL;
        }
        else
        {
            iniciar_fase(jogo, jogo->fase_atual + 1);
        }
    }
}

void jogo_atualizar(Jogo *jogo, Assets *assets, float delta)
{
    if (assets->musica_pronta)
    {
        UpdateMusicStream(assets->musica);
    }

    if (IsKeyPressed(KEY_M))
    {
        jogo->sem_audio = !jogo->sem_audio;
        SetMasterVolume(jogo->sem_audio ? 0.0f : 1.0f);
    }

    bool clicou = entrada_nova_pressao(jogo);
    Vector2 cursor = posicao_entrada_logica();

    if (jogo->estado == ESTADO_MENU)
    {
        atualizar_menu(jogo, assets, clicou, cursor);
        atualizar_particulas(jogo, delta);
        return;
    }

    if (jogo->estado == ESTADO_VITORIA_FASE ||
        jogo->estado == ESTADO_DERROTA ||
        jogo->estado == ESTADO_FINAL)
    {
        atualizar_estado_finalizado(jogo, assets, clicou, cursor);
        atualizar_particulas(jogo, delta);
        return;
    }

    if (IsKeyPressed(KEY_P))
    {
        jogo->estado = jogo->estado == ESTADO_PAUSADO ? ESTADO_JOGANDO : ESTADO_PAUSADO;
    }

    if (jogo->estado == ESTADO_PAUSADO)
    {
        if (clicou && CheckCollisionPointRec(cursor, botao_continuar()))
        {
            tocar_som_seguro(jogo->sem_audio, assets->clique);
            jogo->estado = ESTADO_JOGANDO;
        }
        return;
    }

    if (IsKeyPressed(KEY_R))
    {
        if (jogo->modo == MODO_SOBREVIVENCIA)
        {
            iniciar_onda_sobrevivencia(jogo);
        }
        else
        {
            iniciar_fase(jogo, jogo->fase_atual);
        }
        return;
    }

    if (IsKeyPressed(KEY_ONE))
    {
        trocar_arma_jogador(jogo, ARMA_PISTOLA);
    }
    else if (IsKeyPressed(KEY_TWO))
    {
        trocar_arma_jogador(jogo, ARMA_REVOLVER);
    }
    else if (IsKeyPressed(KEY_THREE))
    {
        trocar_arma_jogador(jogo, ARMA_ESPINGARDA);
    }

    bool clique_em_interface = false;
    if (clicou)
    {
        for (int i = 0; i < TOTAL_ARMAS; i++)
        {
            if (CheckCollisionPointRec(cursor, botao_arma(i)))
            {
                trocar_arma_jogador(jogo, (TipoArma)i);
                tocar_som_seguro(jogo->sem_audio, assets->clique);
                clique_em_interface = true;
                break;
            }
        }
    }

    atualizar_arma(&jogo->jogador, delta);
    atualizar_arma(&jogo->inimigo, delta);

    bool quer_disparar = IsKeyPressed(KEY_SPACE) || (clicou && !clique_em_interface);

    if (quer_disparar)
    {
        arma_disparar(jogo, &jogo->jogador, assets);
    }

    atualizar_inimigo(jogo, assets, delta);

    b2World_Step(jogo->mundo, PASSO_FISICA, SUBPASSOS_FISICA);

    // Eu limito velocidades extremas depois da física para preservar controle e legibilidade.
    estabilizar_arma(&jogo->jogador);
    estabilizar_arma(&jogo->inimigo);

    processar_eventos_contato(jogo, assets);
    atualizar_projeteis(jogo, delta);
    destruir_projeteis_marcados(jogo);
    verificar_powerups(jogo, assets);
    atualizar_particulas(jogo, delta);
    verificar_fim_fase(jogo);
}

static void desenhar_textura_centralizada(
    Texture2D textura,
    Vector2 centro,
    float largura,
    float altura,
    float angulo,
    Color cor
)
{
    if (!IsTextureValid(textura))
    {
        DrawRectanglePro(
            (Rectangle){centro.x, centro.y, largura, altura},
            (Vector2){largura * 0.5f, altura * 0.5f},
            angulo,
            cor
        );
        return;
    }

    Rectangle origem = {0.0f, 0.0f, (float)textura.width, (float)textura.height};
    Rectangle destino = {centro.x, centro.y, largura, altura};
    Vector2 pivo = {largura * 0.5f, altura * 0.5f};
    DrawTexturePro(textura, origem, destino, pivo, angulo, cor);
}

static void desenhar_arma(const Arma *arma, const Assets *assets)
{
    if (arma->vida <= 0 || !b2Body_IsValid(arma->corpo))
    {
        return;
    }

    const ConfigArma *config = &CONFIG_ARMAS[arma->tipo];
    b2Vec2 pos = b2Body_GetPosition(arma->corpo);
    float angulo = b2Rot_GetAngle(b2Body_GetRotation(arma->corpo)) * RAD_PARA_GRAUS;
    Vector2 centro = {m_para_px(pos.x), m_para_px(pos.y)};
    Texture2D textura = assets_textura_arma(assets, arma->tipo);

    desenhar_textura_centralizada(
        textura,
        centro,
        config->largura_visual,
        config->altura_visual,
        angulo,
        WHITE
    );

    if (arma->tempo_flash > 0.0f && IsTextureValid(assets->muzzle))
    {
        Vector2 cano = posicao_cano(arma);
        float tamanho = 44.0f;

        /*
         * O asset do muzzle aponta para cima. Eu prendo a BASE do efeito
         * exatamente no cano e giro +90 graus para alinhá-lo ao eixo +X da arma.
         * Assim o flash acompanha a arma em vez de "ficar para trás".
         */
        Rectangle origem = {
            0.0f,
            0.0f,
            (float)assets->muzzle.width,
            (float)assets->muzzle.height
        };
        Rectangle destino = {
            cano.x,
            cano.y,
            tamanho,
            tamanho
        };
        Vector2 pivo = {
            tamanho * 0.5f,
            tamanho * 0.82f
        };

        DrawTexturePro(
            assets->muzzle,
            origem,
            destino,
            pivo,
            angulo + 90.0f,
            WHITE
        );
    }
}

static void desenhar_projeteis(const Jogo *jogo, const Assets *assets)
{
    for (int i = 0; i < MAX_PROJETEIS; i++)
    {
        const Projetil *projetil = &jogo->projeteis[i];

        if (!projetil->ativo || projetil->remover || !b2Body_IsValid(projetil->corpo))
        {
            continue;
        }

        b2Vec2 pos = b2Body_GetPosition(projetil->corpo);
        b2Vec2 vel = b2Body_GetLinearVelocity(projetil->corpo);
        Vector2 centro = {m_para_px(pos.x), m_para_px(pos.y)};
        float angulo = atan2f(vel.y, vel.x) * RAD_PARA_GRAUS + 90.0f;
        Texture2D textura = assets_textura_bala(assets, projetil->tipo_arma);
        Color cor = projetil->dono == DONO_JOGADOR ? WHITE : (Color){255, 120, 120, 255};

        if (IsTextureValid(textura))
        {
            float altura = projetil->tipo_arma == ARMA_ESPINGARDA ? 16.0f : 18.0f;
            float largura = 6.0f;
            desenhar_textura_centralizada(textura, centro, largura, altura, angulo, cor);
        }
        else
        {
            DrawCircleV(centro, m_para_px(RAIO_PROJETIL_METROS), cor);
        }
    }
}

static Color cor_powerup(TipoPowerup tipo)
{
    switch (tipo)
    {
        case POWERUP_DUPLO:
            return (Color){30, 178, 255, 255};
        case POWERUP_TRIPLO:
            return (Color){180, 70, 245, 255};
        case POWERUP_RICOCHETE:
            return (Color){255, 173, 45, 255};
        case POWERUP_VELOCIDADE:
            return (Color){65, 235, 140, 255};
        case POWERUP_CAVEIRA:
            return (Color){220, 65, 85, 255};
        default:
            return WHITE;
    }
}

static void desenhar_caveira(Rectangle area)
{
    Vector2 centro = {
        area.x + area.width * 0.5f,
        area.y + area.height * 0.43f
    };
    float raio = area.width * 0.25f;

    DrawCircleV(centro, raio, (Color){245, 238, 220, 255});
    DrawRectangle(
        (int)(centro.x - raio * 0.58f),
        (int)(centro.y + raio * 0.45f),
        (int)(raio * 1.16f),
        (int)(raio * 0.62f),
        (Color){245, 238, 220, 255}
    );
    DrawCircle((int)(centro.x - raio * 0.36f), (int)(centro.y - 1.0f), raio * 0.22f, (Color){35, 26, 36, 255});
    DrawCircle((int)(centro.x + raio * 0.36f), (int)(centro.y - 1.0f), raio * 0.22f, (Color){35, 26, 36, 255});
    DrawTriangle(
        (Vector2){centro.x, centro.y + raio * 0.20f},
        (Vector2){centro.x - raio * 0.12f, centro.y + raio * 0.50f},
        (Vector2){centro.x + raio * 0.12f, centro.y + raio * 0.50f},
        (Color){35, 26, 36, 255}
    );
}

static void desenhar_powerups(const Jogo *jogo, const Assets *assets)
{
    for (int i = 0; i < jogo->quantidade_powerups; i++)
    {
        const Powerup *powerup = &jogo->powerups[i];

        if (!powerup->ativo)
        {
            continue;
        }

        Color base = cor_powerup(powerup->tipo);
        float brilho = 0.86f + 0.14f * sinf(powerup->pulso);
        Color fundo = {
            (unsigned char)(base.r * brilho),
            (unsigned char)(base.g * brilho),
            (unsigned char)(base.b * brilho),
            235
        };

        if (IsTextureValid(assets->brilho_powerup))
        {
            Rectangle src = {0, 0, (float)assets->brilho_powerup.width, (float)assets->brilho_powerup.height};
            Rectangle dst = {
                powerup->area.x - 11.0f,
                powerup->area.y - 11.0f,
                powerup->area.width + 22.0f,
                powerup->area.height + 22.0f
            };
            DrawTexturePro(assets->brilho_powerup, src, dst, (Vector2){0, 0}, 0.0f, Fade(base, 0.38f));
        }

        DrawRectangleRounded(powerup->area, 0.22f, 8, fundo);
        DrawRectangleRoundedLinesEx(powerup->area, 0.22f, 8, 3.0f, RAYWHITE);

        if (powerup->tipo == POWERUP_CAVEIRA)
        {
            desenhar_caveira(powerup->area);
        }
        else
        {
            const char *texto = nome_powerup(powerup->tipo);
            int tamanho = powerup->tipo == POWERUP_VELOCIDADE ? 15 : 24;
            int largura = MeasureText(texto, tamanho);
            DrawText(
                texto,
                (int)(powerup->area.x + powerup->area.width * 0.5f - largura * 0.5f),
                (int)(powerup->area.y + powerup->area.height * 0.5f - tamanho * 0.5f),
                tamanho,
                RAYWHITE
            );
        }
    }
}

static void desenhar_particulas(const Jogo *jogo, const Assets *assets)
{
    for (int i = 0; i < MAX_PARTICULAS; i++)
    {
        const Particula *p = &jogo->particulas[i];

        if (!p->ativo)
        {
            continue;
        }

        float alpha = limitar(p->vida / p->vida_total, 0.0f, 1.0f);
        Color cor = Fade(p->cor, alpha);

        if (IsTextureValid(assets->faisca))
        {
            float tamanho = p->tamanho * 2.0f;
            desenhar_textura_centralizada(
                assets->faisca,
                p->posicao,
                tamanho,
                tamanho,
                0.0f,
                cor
            );
        }
        else
        {
            DrawCircleV(p->posicao, p->tamanho * 0.5f, cor);
        }
    }
}

static void desenhar_barra_vida(float x, float y, float largura, const Arma *arma, Color cor)
{
    float proporcao = arma->vida_maxima > 0 ? (float)arma->vida / arma->vida_maxima : 0.0f;
    proporcao = limitar(proporcao, 0.0f, 1.0f);

    DrawRectangleRounded((Rectangle){x, y, largura, 12.0f}, 0.4f, 5, (Color){50, 52, 62, 255});
    DrawRectangleRounded((Rectangle){x, y, largura * proporcao, 12.0f}, 0.4f, 5, cor);
}

static void desenhar_hud(const Jogo *jogo)
{
    DefinicaoFase fase = fase_obter(jogo->fase_atual);

    DrawText(TextFormat("FASE %d/%d", jogo->fase_atual + 1, TOTAL_FASES), 20, 14, 18, RAYWHITE);
    DrawText(fase.nome, 20, 36, 14, GRAY);
    DrawText(TextFormat("PONTOS %05d", jogo->pontuacao), 20, 61, 16, YELLOW);

    desenhar_barra_vida(178.0f, 18.0f, 125.0f, &jogo->jogador, (Color){60, 220, 135, 255});
    desenhar_barra_vida(178.0f, 40.0f, 125.0f, &jogo->inimigo, (Color){238, 78, 92, 255});
    DrawText("VOCÊ", 308, 15, 11, (Color){60, 220, 135, 255});
    DrawText("RIVAL", 308, 37, 11, (Color){238, 78, 92, 255});

    for (int i = 0; i < TOTAL_ARMAS; i++)
    {
        Rectangle botao = botao_arma(i);
        bool selecionada = jogo->arma_selecionada == (TipoArma)i;
        DrawRectangleRounded(
            botao,
            0.25f,
            6,
            selecionada ? (Color){70, 88, 128, 255} : (Color){42, 45, 57, 255}
        );
        DrawRectangleRoundedLinesEx(botao, 0.25f, 6, 1.5f, selecionada ? SKYBLUE : DARKGRAY);
        DrawText(TextFormat("%d", i + 1), (int)botao.x + 5, (int)botao.y + 7, 12, GRAY);
        DrawText(
            i == ARMA_PISTOLA ? "P" : (i == ARMA_REVOLVER ? "R" : "S"),
            (int)botao.x + 31,
            (int)botao.y + 6,
            14,
            selecionada ? RAYWHITE : LIGHTGRAY
        );
    }

    DrawText(
        TextFormat("BALAS %d/%d", contar_projeteis(jogo, -1), MAX_PROJETEIS),
        415,
        86,
        11,
        GRAY
    );
}

static void desenhar_arena(const Jogo *jogo)
{
    DrawRectangleRec(jogo->arena, (Color){21, 23, 31, 255});

    for (int y = 120; y < 910; y += 40)
    {
        DrawLine(32, y, 508, y, (Color){30, 33, 43, 255});
    }

    for (int x = 52; x < 508; x += 40)
    {
        DrawLine(x, 112, x, 908, (Color){30, 33, 43, 255});
    }

    DrawRectangleRec(jogo->parede_esquerda, (Color){75, 79, 94, 255});
    DrawRectangleRec(jogo->parede_direita, (Color){75, 79, 94, 255});
    DrawRectangleRec(jogo->teto, (Color){75, 79, 94, 255});
    DrawRectangleRec(jogo->chao, (Color){75, 79, 94, 255});

    for (int i = 0; i < jogo->quantidade_obstaculos; i++)
    {
        Rectangle obstaculo = jogo->obstaculos[i].area;
        DrawRectangleRounded(obstaculo, 0.18f, 4, (Color){91, 95, 112, 255});
        DrawRectangleRoundedLinesEx(obstaculo, 0.18f, 4, 1.0f, (Color){128, 132, 148, 255});
    }
}

static void desenhar_overlay(const char *titulo, const char *subtitulo, const char *botao)
{
    DrawRectangle(0, 0, LARGURA_LOGICA, ALTURA_LOGICA, Fade(BLACK, 0.68f));
    int largura_titulo = MeasureText(titulo, 38);
    DrawText(titulo, LARGURA_LOGICA / 2 - largura_titulo / 2, 430, 38, RAYWHITE);

    int largura_sub = MeasureText(subtitulo, 18);
    DrawText(subtitulo, LARGURA_LOGICA / 2 - largura_sub / 2, 485, 18, LIGHTGRAY);

    Rectangle area = botao_continuar();
    DrawRectangleRounded(area, 0.18f, 8, (Color){67, 88, 140, 255});
    DrawRectangleRoundedLinesEx(area, 0.18f, 8, 2.0f, SKYBLUE);
    int largura_botao = MeasureText(botao, 20);
    DrawText(
        botao,
        (int)(area.x + area.width * 0.5f - largura_botao * 0.5f),
        (int)(area.y + 21.0f),
        20,
        RAYWHITE
    );
}

static void desenhar_menu(const Jogo *jogo, const Assets *assets)
{
    ClearBackground((Color){15, 17, 24, 255});

    DrawCircleGradient((Vector2){270.0f, 210.0f}, 220.0f, (Color){48, 68, 115, 100}, BLANK);
    DrawText("RECOIL", 92, 135, 68, RAYWHITE);
    DrawText("GUN", 325, 135, 68, (Color){248, 99, 91, 255});
    const char *descricao_1 = "Jogo criado para estudar C, física, vetores,";
    const char *descricao_2 = "cálculos, colisões e lógica de programação.";

    int largura_descricao_1 = MeasureText(descricao_1, 16);
    int largura_descricao_2 = MeasureText(descricao_2, 16);

    DrawText(
        descricao_1,
        LARGURA_LOGICA / 2 - largura_descricao_1 / 2,
        222,
        16,
        SKYBLUE
    );
    DrawText(
        descricao_2,
        LARGURA_LOGICA / 2 - largura_descricao_2 / 2,
        247,
        16,
        SKYBLUE
    );

    DrawText("ESCOLHA A ARMA", 175, 410, 19, RAYWHITE);

    for (int i = 0; i < TOTAL_ARMAS; i++)
    {
        Rectangle carta = {70.0f + i * 140.0f, 465.0f, 120.0f, 150.0f};
        bool selecionada = jogo->arma_selecionada == (TipoArma)i;
        Color fundo = selecionada ? (Color){54, 70, 110, 255} : (Color){31, 34, 45, 255};
        DrawRectangleRounded(carta, 0.12f, 8, fundo);
        DrawRectangleRoundedLinesEx(carta, 0.12f, 8, 2.0f, selecionada ? SKYBLUE : DARKGRAY);

        Texture2D textura = assets_textura_arma(assets, (TipoArma)i);
        const ConfigArma *config = &CONFIG_ARMAS[i];
        float largura = i == ARMA_ESPINGARDA ? 104.0f : 76.0f;
        float altura = i == ARMA_ESPINGARDA ? 30.0f : 58.0f;
        desenhar_textura_centralizada(
            textura,
            (Vector2){carta.x + 60.0f, carta.y + 55.0f},
            largura,
            altura,
            0.0f,
            WHITE
        );

        int largura_nome = MeasureText(config->nome, 12);
        DrawText(config->nome, (int)(carta.x + 60.0f - largura_nome * 0.5f), (int)carta.y + 104, 12, RAYWHITE);
        DrawText(TextFormat("[%d]", i + 1), (int)carta.x + 49, (int)carta.y + 127, 12, GRAY);
    }

    Rectangle jogar = botao_jogar();
    DrawRectangleRounded(jogar, 0.18f, 8, (Color){210, 70, 78, 255});
    DrawRectangleRoundedLinesEx(jogar, 0.18f, 8, 2.0f, (Color){255, 135, 130, 255});
    DrawText("CAMPANHA", 199, 698, 21, RAYWHITE);

    Rectangle sobrevivencia = botao_sobrevivencia();
    DrawRectangleRounded(sobrevivencia, 0.18f, 8, (Color){58, 103, 151, 255});
    DrawRectangleRoundedLinesEx(sobrevivencia, 0.18f, 8, 2.0f, SKYBLUE);
    DrawText("SOBREVIVÊNCIA", 181, 766, 20, RAYWHITE);

    DrawText("ENTER = campanha | S = sobrevivência", 140, 820, 13, LIGHTGRAY);
    DrawText("PC: ESPAÇO/clique = tiro | 1 2 3 = arma | P = pausa", 58, 848, 13, GRAY);
    DrawText("Mobile: toque = tiro | toque nos botões = trocar arma", 55, 871, 13, GRAY);
    DrawText("M = áudio | R = reiniciar", 173, 898, 13, GRAY);
}

void jogo_desenhar(const Jogo *jogo, const Assets *assets)
{
    if (jogo->estado == ESTADO_MENU)
    {
        desenhar_menu(jogo, assets);
        desenhar_particulas(jogo, assets);
        return;
    }

    ClearBackground((Color){14, 16, 22, 255});
    desenhar_hud(jogo);
    desenhar_arena(jogo);
    desenhar_powerups(jogo, assets);
    desenhar_projeteis(jogo, assets);
    desenhar_arma(&jogo->jogador, assets);
    desenhar_arma(&jogo->inimigo, assets);
    desenhar_particulas(jogo, assets);

    if (jogo->estado == ESTADO_PAUSADO)
    {
        desenhar_overlay("PAUSADO", "P para voltar", "CONTINUAR");
    }
    else if (jogo->estado == ESTADO_VITORIA_FASE)
    {
        if (jogo->fase_atual + 1 >= TOTAL_FASES)
        {
            desenhar_overlay("VITÓRIA!", "Você venceu o último duelo.", "VER RESULTADO");
        }
        else
        {
            desenhar_overlay("FASE LIMPA!", "O próximo duelo será mais caótico.", "PRÓXIMA FASE");
        }
    }
    else if (jogo->estado == ESTADO_DERROTA)
    {
        desenhar_overlay("DERROTA", "Ajuste o tempo dos disparos e tente de novo.", "REPETIR FASE");
    }
    else if (jogo->estado == ESTADO_FINAL)
    {
        DrawRectangle(0, 0, LARGURA_LOGICA, ALTURA_LOGICA, Fade(BLACK, 0.78f));
        DrawText("CAMPANHA CONCLUÍDA", 72, 370, 30, RAYWHITE);
        DrawText(TextFormat("PONTUAÇÃO FINAL: %d", jogo->pontuacao), 135, 425, 20, YELLOW);
        DrawText("Você dominou o recuo, os ricochetes e o caos.", 62, 470, 16, LIGHTGRAY);
        Rectangle area = botao_continuar();
        DrawRectangleRounded(area, 0.18f, 8, (Color){67, 88, 140, 255});
        DrawText("VOLTAR AO MENU", 188, 677, 18, RAYWHITE);
    }

    if (jogo->sem_audio)
    {
        DrawText("MUDO", 475, 925, 10, GRAY);
    }
}
