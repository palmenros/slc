/*
 * Copyright (C) 2023 Pedro Palacios Almendros, Ricardo Maurizio Paul
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef COMPILER_TYPE_H
#define COMPILER_TYPE_H

#include "../utils/log.hpp"
#include "llvm/Support/Casting.h"
#include <string>
#include <string_view>
#include <unordered_map>

namespace Ceres {

    namespace Typing {
        class TypeVisitor;
    }

    // Note: Types are cached. There is only a unique instance per type. Types should have a private
    // constructor and have a get() function that performs the cache lookup
    class Type {
    public:
        /// Discriminator for LLVM-style RTTI (dyn_cast<> et al.)
        // TODO: When adding a new type kind, you also have to add it here
        enum class TypeKind {
            UnitVoidType,
            UnresolvedType,
            NotYetInferredType,
            PrimitiveScalarType
        };

    private:
        const TypeKind kind;

    public:
        explicit Type(TypeKind kind) : kind(kind) {}
        TypeKind getKind() const { return kind; }

        virtual void accept(Typing::TypeVisitor &visitor) = 0;
        virtual std::string toString() const = 0;
        virtual ~Type() = default;
    };

    class UnitVoidType : public Type {
    private:
        static std::unique_ptr<UnitVoidType> instance;
        UnitVoidType() : Type(TypeKind::UnitVoidType) {}

    public:
        // Static function needed for fast LLVM RTTI
        static bool classof(const Type *type) { return type->getKind() == TypeKind::UnitVoidType; }
        void accept(Typing::TypeVisitor &visitor) override;

    public:
        static UnitVoidType *get();
        std::string toString() const override;
    };

    // UnresolvedType is a type that hasn't been resolved yet. For example, it is a user-created type
    //  and we haven't run yet type-binding, so we don't have fully resolved this type.
    class UnresolvedType : public Type {
    private:
        static std::unordered_map<std::string, std::unique_ptr<UnresolvedType>> instances;
        explicit UnresolvedType(std::string typeIdentifier);

    public:
        // Static function needed for fast LLVM RTTI
        static bool classof(const Type *type) {
            return type->getKind() == TypeKind::UnresolvedType;
        }
        void accept(Typing::TypeVisitor &visitor) override;

    public:
        std::string typeIdentifier;

        // TODO: Consider using std::string_view instead of std::string here and everywhere else
        //          where it is appropriate.
        static UnresolvedType *get(const std::string &str);
        std::string toString() const override;
    };

    // NotYetInferredKind is a type that hasn't been inferred yet and has not been specified by the user
    // Examples:
    //      Number literal without a specified type
    //      Variable declaration without a user-specified type
    enum class NotYetInferredKind {
        NumberLiteral,      // Number literal without a specified type such as '2' instead of '2u32'
        VariableDeclaration,// Variable declaration without a specified type such as 'const i = 1u32';
        Expression,         //
    };

    class NotYetInferredType : public Type {
    private:
        static std::unordered_map<NotYetInferredKind, std::unique_ptr<NotYetInferredType>>
                instances;
        explicit NotYetInferredType(NotYetInferredKind kind)
            : kind(kind), Type(TypeKind::NotYetInferredType) {}

    public:
        // Static function needed for fast LLVM RTTI
        static bool classof(const Type *type) {
            return type->getKind() == TypeKind::NotYetInferredType;
        }
        void accept(Typing::TypeVisitor &visitor) override;

    public:
        NotYetInferredKind kind;

        static NotYetInferredType *get(NotYetInferredKind kind);
        std::string toString() const override;
    };

    enum class PrimitiveKind {
        // signed
        I8,
        I16,
        I32,
        I64,
        // unsigned
        U8,
        U16,
        U32,
        U64,
        // float
        F32,
        F64,
    };

    class PrimitiveScalarType : public Type {
    private:
        static std::unordered_map<PrimitiveKind, std::unique_ptr<PrimitiveScalarType>> instances;
        explicit PrimitiveScalarType(PrimitiveKind kind);

    public:
        // Static function needed for fast LLVM RTTI
        static bool classof(const Type *type) {
            return type->getKind() == TypeKind::NotYetInferredType;
        }
        void accept(Typing::TypeVisitor &visitor) override;

    public:
        PrimitiveKind kind;

        static PrimitiveKind primitiveKindFromString(std::string_view str);

        static PrimitiveScalarType *get(PrimitiveKind kind);
        static PrimitiveScalarType *get(std::string_view str);
        std::string toString() const override;
    };


}// namespace Ceres

#endif//COMPILER_TYPE_H
