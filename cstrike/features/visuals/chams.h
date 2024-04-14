#pragma once

#pragma once

#include "../../common.h"
#include <memory>
#include "../cstrike/sdk/datatypes/stronghandle.hpp"
#pragma once
class material_data_t;
class material2_t;
namespace ams {
    class chams_t {
    public:
        bool initialize();
        void destroy();
    public:
        bool draw_object(void* animatable_object, void* dx11, material_data_t* arr_material_data, int data_count,
            void* scene_view, void* scene_layer, void* unk1, void* unk2);

        bool override_material(void* animatable_object, void* dx11, material_data_t* arr_material_data, int data_count,
            void* scene_view, void* scene_layer, void* unk1, void* unk2);
    public: 
        material2_t* CreateMaterial(const char* szName, const char* szMaterialVMAT, const char* szShaderType, bool bBlendMode, bool bTranslucent, bool bDisableZBuffering);

        material2_t* create_material(const char* m_name, const char* material_vmat, const char* shader_type, bool blend_mode,
            bool transfluscent, bool disable_buffering);
    private:
        bool m_initialized = false;
    };
    const auto chams = std::make_unique<chams_t>();


}