/*
 * ===================== CircuitBoard.cpp ==========================
 *                          -- tpr --
 *                                        CREATE -- 2020.01.04
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "CircuitBoard.h"


//------------------- Engine --------------------//
#include "GameObj.h"
#include "esrc_gameObj.h"



//===== static =====//
std::unordered_map<mapEntKey_t, std::unique_ptr<CircuitBoard::MapEntMessage>> CircuitBoard::messages {};
std::unordered_map<goid_t, F_AFFECT> CircuitBoard::functors {};



void CircuitBoard::init_for_static()noexcept{
    CircuitBoard::messages.reserve( 1000 ); // 随便某个值，未测试
    CircuitBoard::functors.reserve( 1000 ); // 随便某个值，未测试
}


// dogo 主动登记 msg 函数 
void CircuitBoard::signUp(  goid_t dogoid_, F_AFFECT functor_,
                            const std::map<mapEntKey_t, CircuitBoard::MessageWeight> &mpDatas_ )noexcept{

    auto outPair1 = CircuitBoard::functors.insert({ dogoid_, functor_ });
    tprAssert( outPair1.second ); // 一个 dogo 只能登记 一个 functor ！
    //---
    mapEntKey_t mpKey {};
    MessageWeight weight {};
    for( const auto &ipair : mpDatas_ ){
        mpKey = ipair.first;
        weight = ipair.second;

        auto outPair2 = CircuitBoard::messages.insert({ mpKey, std::make_unique<MapEntMessage>() });// insert or find
        MapEntMessage &mpMsgRef = *(outPair2.first->second);
        mpMsgRef.dogoids.insert({ weight, dogoid_ }); // always can
    }
}




// 每个 mpgo，在自己被创建出来后，都要主动调用本函数，检查目标 
void CircuitBoard::check_and_call_messages( IntVec2 mpos_, GameObj &begoRef_ )noexcept{

    mapEntKey_t mpKey = mpos_2_key(mpos_);
    if( auto target=CircuitBoard::messages.find(mpKey); target!=CircuitBoard::messages.end() ){

        MapEntMessage &mpMsgRef = *(target->second);
        goid_t dogoid {};

        for( auto rit=mpMsgRef.dogoids.rbegin(); rit!=mpMsgRef.dogoids.rend(); rit++ ){
            dogoid = rit->second;

            // dogo Must Existed 
            tprAssert( esrc::is_go_active(dogoid) );
            GameObj &dogoRef = esrc::get_goRef(dogoid);

            // call message functor
            auto it = CircuitBoard::functors.find(dogoid);
            tprAssert( it != CircuitBoard::functors.end() );

            it->second( dogoRef, begoRef_ );
        }
    }
}



void CircuitBoard::erase_dogoMessages(  goid_t dogoid_, 
                                        const std::map<mapEntKey_t, CircuitBoard::MessageWeight> &mpDatas_ )noexcept{

    size_t eraseNum = CircuitBoard::functors.erase( dogoid_ );
    tprAssert( eraseNum == 1 ); // Must Have

    mapEntKey_t mpKey {};
    MessageWeight weight {};
    for( const auto &ipair : mpDatas_ ){ // each mpData
        mpKey = ipair.first;
        weight = ipair.second;

        auto it = CircuitBoard::messages.find( mpKey );
        tprAssert( it != CircuitBoard::messages.end() ); // Must Have

        MapEntMessage &mpMsgRef = *(it->second);

        // 比较累的操作，手动从 mmap 容器中找到 目标元素，删除之
        for( auto fit=mpMsgRef.dogoids.find(weight); fit!=mpMsgRef.dogoids.end(); fit++ ){
            if( fit->first != weight ){
                break;
            }
            if( fit->second == dogoid_ ){
                mpMsgRef.dogoids.erase(fit);
                break;
            }
        }

        // 如果目标 mapEntMessage.dogoids 已经空了
        //  直接删除 本 MapEntMessage 实例 
        if( mpMsgRef.dogoids.empty() ){
            eraseNum = CircuitBoard::messages.erase( mpKey );
            tprAssert( eraseNum == 1 ); // Must Have
            continue; // Next !!!
        }
    }
}



