
#include "schemav2.hpp"
#include "../sdk/interfaces/ischemasystem.h"
#include "../utilities/fnv1a.h"
#include "../utilities/log.h"

std::optional<int32_t> CSchemaManager::GetSchemaOffsetInternal(const char* moduleName, const char* bindingName, const char* fieldName) {
    CSchemaSystemTypeScope* typeScope = I::SchemaSystem->FindTypeScopeForModule(moduleName);
    if (!typeScope) {
        L_PRINT(LOG_ERROR) << CS_XOR("\"schemamgr\" No type scope found for " << moduleName);
        return {};
    }
    SchemaClassInfoData_t* classInfo;

    typeScope->FindDeclaredClass(&classInfo, bindingName);
    if (!classInfo) {
        L_PRINT(LOG_INFO) << CS_XOR("\"schemamgr\" No binding named '{}' has been found in module '{}'.");

        return {};
    }

    uint32_t fieldHash = FNV1A::Hash(fieldName);
    for (int i = 0; classInfo->pFields && i < classInfo->nFieldSize; ++i) {
        auto& field = classInfo->pFields[i];
        if (FNV1A::Hash(field.szName) == fieldHash) {
            L_PRINT(LOG_WARNING) << CS_XOR("\"schemamgr\" offset" << bindingName << "field: " << fieldName);

            return field.nSingleInheritanceOffset;
        }
    }
    L_PRINT(LOG_ERROR) << CS_XOR("\"schemamgr\"  No field named " << fieldName << "in binding: " << bindingName);

    return {};
}
