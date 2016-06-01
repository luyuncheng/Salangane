//
// Created by root on 16-4-2.
//
// 一个空基类，用于标识 (tag) 值类型
#ifndef SALANGANE_COPYABLE_H
#define SALANGANE_COPYABLE_H
namespace salangane {
    /// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
    class copyable{

    };
};
#endif //SALANGANE_COPYABLE_H
