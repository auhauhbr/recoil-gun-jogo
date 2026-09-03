#ifndef RECOIL_ASSETS_H
#define RECOIL_ASSETS_H

#include "tipos.h"

Assets assets_carregar(void);
void assets_descarregar(Assets *assets);
Texture2D assets_textura_arma(const Assets *assets, TipoArma tipo);
Texture2D assets_textura_bala(const Assets *assets, TipoArma tipo);
Sound assets_som_tiro(const Assets *assets, TipoArma tipo);

#endif
