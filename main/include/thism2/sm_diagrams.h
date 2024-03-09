//
// Created by dheide on 22.11.21.
//

#ifndef THISM2_SM_DIAGRAMS_H
#define THISM2_SM_DIAGRAMS_H

#include "thism2/sm.h"

#include <iostream>
#include <qstring.h>
#include <qvector.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>


namespace helper {
    template <typename ...>
    struct StateLevelList;
    template <typename ... STATES>
    struct StateLevelList<Collector<STATES...>> {
        uint16_t stateLevel[sizeof...(STATES)];

        StateLevelList() : stateLevel { sys_detail::StateLevel<STATES>::type::value ... }
        { }
    };

    template <typename ...>
    struct StateList_States;
    template <typename SYS, typename ... STATEs>
    struct StateList_States<SYS, Collector<STATEs...>> {
        QVector<uint16_t> ids;

        StateList_States() : ids { SYS::StateListT::template StateId<STATEs>::type::value ... }
        { }
    };
}

template <typename SM, typename SYS>
QString make_treeuml(SYS *sys) {
#ifndef __useDescription
    static_assert(false, "define switch __useDescription has to be activated.");
#endif
    //BAHA_TYPE *baha = sys->hwalBaseGet();

    HWAL_Log *lol = sys->hwalBaseGet()->logger_get();
    QString stateUML[SYS::numberOfStatesT::value];
    QString out;

    helper::StateLevelList<typename SYS::StatesT> sll;

    helper::StateList_States<SYS, typename SM::StatesT> sls;

    for(uint16_t cl = sys_detail::MaxStateLevelC<typename SYS::StatesT>::type::value; cl > 0; cl--)
        for (uint16_t cs = 0; cs != SYS::numberOfStatesT::value; cs++)
            if ((sll.stateLevel[cs] == cl) && (sls.ids.contains(cs))) {
                //lol->logf(HWAL_Log::Always, "Level: %d: ", (uint8_t) cl, ": ", sys->getStateName(cs) ); //StateIdT{cs});
                std::cout << QString("Level: %1 - %2 (%3)\n").arg((int) cl).arg(sys->getStateName(cs))
                    .arg(cs).toStdString();

                uint16_t pid = sys->getParentIdBI(cs);

                QString nt = sys->getStateById(cs)->name;
                QString s = QString("state \"%1 (%2)\" as %1 {\n").arg(nt).arg(cs);
                QString cl;
                foreach(cl, stateUML[cs].split("\n", Qt::SkipEmptyParts)) {
                    s += QString("  %1\n").arg(cl);
                }

                {
//                    TransitionImpl *trs = sys->transitions[cs];
//                    for(uint16_t ct=0; ct!=sys->transitionsNumberPerState[cs]; ct++)
                    sys_detail::TransitionsForState tfs = sys->transitionsForStateGetBI(cs);
                    for (uint16_t ct = 0; ct != tfs.transitionNum; ct++)
                        if (sls.ids.contains(cs)) {
                            if (sys->checkIfStateIsChildOfOrSame(cs, tfs.transitions[ct].stateId)) {
                                s += QString("[*]");
                            } else {
                                s += QString("%1").arg(sys->getStateById(cs)->name);
                            }
                            s += QString(" --> %1: %2 (%3)\\n%4\n")
                                    .arg(sys->getStateById(tfs.transitions[ct].stateId)->name)
                                    .arg(event_details::getEventName<typename SYS::EventListT>(
                                            tfs.transitions[ct].eventId))
                                    .arg(tfs.transitions[ct].eventId)
                                    .arg((smsys->eventOptsGetById(tfs.transitions[ct].eventId) &
                                        EOPT_IGNORE_IF_DEST_STATE_IS_ACTIVE) > 0 ? "[ignore_if_dest_state_is_active]" : "");
                        }
                }

                s += "}\n";

                QString ds = sys->getStateById(cs)->description;
                QStringList sl = ds.split("\n", Qt::SkipEmptyParts);
                foreach(cl, sl) {
                    s += QString("%1 : %2\n").arg(nt).arg(cl);
                }
                if (pid != SYS::ID_S_Undefined)
                    stateUML[pid] += s;
                else
                    out += s;
            }


    out += QString("\n[*] --> %1\n").arg(sys->template getState<typename SM::InitialStateT>()->name);

//    for(uint16_t cs=0; cs!=SYS::numberOfStatesT::value; cs++) {
//        TransitionImpl *trs = sys->transitions[cs];
//        for(uint16_t ct=0; ct!=sys->transitionsNumberPerState[cs]; ct++)
//            if(sls.ids.contains(cs)) {
//                out += QString("%1 --> %2: %3\n").arg(sys->getStateById(cs)->name())
//                        .arg(sys->getStateById(trs[ct].stateId)->name())
//                        .arg(event_details::getEventName<typename SYS::EventListT>(trs[ct].eventId));
//            }
//    }

    std::cout << out.toStdString();

    return out;
}

#include <QJsonArray>
namespace make_statelist_helper {
    template<typename ... > struct AppendStateName;
    template<typename SYS, typename STATE, typename ... STATEs>
    struct AppendStateName<SYS, Collector<STATE, STATEs...>> {
        void operator()(SYS *sys, QJsonArray &states) {
            states.append(sys->template getStateRef<STATE>().name);
            AppendStateName<SYS, Collector<STATEs...>> asn;
            asn(sys, states);
        }
    };
    template<typename SYS, typename STATE, typename ... STATEs>
    struct AppendStateName<SYS, Collector<MarkInitialState<STATE>, STATEs...>> {
        void operator()(SYS *sys, QJsonArray &states) {
            states.append(QString("*%1").arg(sys->template getStateRef<STATE>().name));
            AppendStateName<SYS, Collector<STATEs...>> asn;
            asn(sys, states);
        }
    };
    template<typename SYS>
    struct AppendStateName<SYS, Collector<>> {
        void operator()(SYS *, QJsonArray &) { }
    };

}
template <typename SM, typename SYS>
QJsonArray make_statelist(SYS *sys) {
    QJsonArray states;

    make_statelist_helper::AppendStateName<SYS, typename SM::StatesWithInitialFlagT> asn;
    asn(sys, states);

    return states;
}

namespace helper {

    template<typename ...>
    struct Make_Treeuml_allSMs_impl;
    template<typename SYS, typename SM, typename ... SMs>
    struct Make_Treeuml_allSMs_impl<SYS, Collector<SM, SMs...>> {
        QString operator()(SYS *sys, const char *path) {
            QString uml_main_part = make_treeuml<SM>(sys);
            QString uml_str = QString("@startuml\n\ntitle %1\n\n%2\n\n@enduml\n")
                    .arg(SM::name()).arg(uml_main_part);

            QFile file(QString("%1/%2.uml").arg(path).arg(SM::name()));
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                return QString(" - ERROR for %1 - ").arg(SM::name());

            QTextStream out(&file);
            out << uml_str;

            Make_Treeuml_allSMs_impl<SYS, Collector<SMs...>> mtu;
            return QString("%1.uml %2").arg(SM::name()).arg(mtu(sys, path));
        }
    };
    template<typename SYS>
    struct Make_Treeuml_allSMs_impl<SYS, Collector<>> {
        QString operator()(SYS */*sys*/, const char *) { return ""; }
    };

    template<typename ...>
    struct Make_StateList_impl;
    template<typename SYS, typename SM, typename ... SMs>
    struct Make_StateList_impl<SYS, Collector<SM, SMs...>> {
        void operator()(QJsonObject &root, SYS *sys) {
            auto sl = make_statelist<SM, SYS>(sys);
            root[SM::name()] = sl;

            Make_StateList_impl<SYS, Collector<SMs...>> msl;
            msl(root, sys);
        }
    };
    template<typename SYS>
    struct Make_StateList_impl<SYS, Collector<>> {
        void operator()(QJsonObject &, SYS *) { }
    };
}

#include <iostream>

template <typename SYS>
void make_treeuml_allSMs(SYS *sys, const char *path) {
    helper::Make_Treeuml_allSMs_impl<SYS, typename SYS::SMsT> mtu;

    std::cout << QDir::current().path().toStdString() << "\n";

    QDir::current().mkpath(path);

    QString pngs = mtu(sys, path);

    QString makefile = QString("PUML    = plantuml\nUMLS = %1\n\n"
                               "all: hello\n\n"
                               "hello:\n\t@echo \"building\" $(UMLS)\n"
                               "\t$(PUML) -tsvg -progress $(UMLS)\n\t@echo\n"
                               "\t@echo \"done.\"\n\n"
                               "clean:\n\t@echo \"cleaning ...\"\n"
                               "\t@rm *.svg 2>/dev/null\n\t@echo \"done.\"\n").arg(pngs);

    QFile file(QString("%1/makefile").arg(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << makefile;
}

#include <QJsonObject>
#include <QJsonDocument>

template <typename SYS>
void make_allSMs_list(SYS *sys, const char *path) {
    QJsonObject root;
    helper::Make_StateList_impl<SYS, typename SYS::SMsT> msl;
    msl(root, sys);

    QByteArray ba = QJsonDocument(root).toJson();
    QTextStream ts(stdout);
    ts << "rendered JSON" << Qt::endl;
    ts << ba;
    {
        QFile fout(QString("%1/state_names.json").arg(path));
        fout.open(QIODevice::WriteOnly);
        fout.write(ba);
    }
}


#endif //THISM2_SM_DIAGRAMS_H
