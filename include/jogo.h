#ifndef RECOIL_JOGO_H
#define RECOIL_JOGO_H

#include "tipos.h"

void jogo_inicializar(Jogo *jogo);
void jogo_finalizar(Jogo *jogo);
void jogo_atualizar(Jogo *jogo, Assets *assets, float delta);
void jogo_desenhar(const Jogo *jogo, const Assets *assets);

#endif
