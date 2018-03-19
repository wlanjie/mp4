//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_ATOMFACTORY_H
#define MP4_ATOMFACTORY_H


#include "atom.h"
#include "array.h"

class AtomFactory {
public:
    // types
    class TypeHandler {
    public:
        virtual ~TypeHandler() {};

        virtual Result createAtom(Atom::Type type,
                                  UI32 size,
                                  ByteStream &stream,
                                  Atom::Type context,
                                  Atom *&atom) = 0;
    };

    // constructor
    AtomFactory() {}

    // destructor
    virtual ~AtomFactory();

    // methods
    Result addTypeHandler(TypeHandler *handler);

    Result removeTypeHandler(TypeHandler *handler);

    Result createAtomFromStream(ByteStream &stream,
                                LargeSize &bytesAvailable,
                                Atom *&atom);

    virtual Result createAtomFromStream(ByteStream &stream,
                                        UI32 type,
                                        UI32 size32,
                                        UI64 size64,
                                        Atom *&atom);

    Result createAtomFromStream(ByteStream &stream,
                                Atom *&atom);

    Result createAtomsFromStream(ByteStream &stream,
                                 AtomParent &atoms);

    Result createAtomsFromStream(ByteStream &stream,
                                 LargeSize bytes_available,
                                 AtomParent &atoms);

    // context
    void pushContext(Atom::Type context);

    void popContext();

    Atom::Type getContext(Ordinal depth = 0);

private:
    // members
    Array<Atom::Type> m_ContextStack;
    List<TypeHandler> m_TypeHandlers;
};

class DefaultAtomFactory : public AtomFactory {
public:
    // class members
    static DefaultAtomFactory Instance_;

    // constructor
    DefaultAtomFactory();

    // this member is used to detect the situation where the 
    // platform's code loader does not construct static C++ objects
    bool initialized;

    Result initialize();
};

#endif //MP4_ATOMFACTORY_H
