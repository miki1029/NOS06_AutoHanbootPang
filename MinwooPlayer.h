#pragma once
#include "IPlayer.h"
#include <vector>
#include <stack>
#include <set>
#include <memory>
#include <queue>
#include <algorithm>
#include <iostream>

using namespace std;

namespace SearchDirection
{
    enum Enum
    {
        Left, Right, Up, Down,
        LeftUp, LeftDown, RightUp, RightDown
    };
}

class MinwooPlayer :
    public IPlayer
{
private:
    int width, height;
    int gemTypeCount, totalTurnCount;
    int* board;
    int boardSize;

public:
    MinwooPlayer();
    ~MinwooPlayer();

    virtual void PrepareGame(
        int width, int height,
        int gemTypeCount, int totalTurnCount);

    virtual int Play(int phase, int* board, int* result);

private:
    struct Point {
        int x, y;
        bool operator==(const Point arg) const { return x == arg.x && y == arg.y; }
        bool operator!=(const Point arg) const { return !operator==(arg); }
        bool operator>(const Point arg) const { return x == arg.x ? y > arg.y : x > arg.x; }
        bool operator>=(const Point arg) const { return operator==(arg) || operator>(arg); }
        bool operator<(const Point arg) const { return x == arg.x ? y < arg.y : x < arg.x; }
        bool operator<=(const Point arg) const { return operator==(arg) || operator<(arg); }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 상태 관리
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Memento;
    // 한 붓 그리기 상태를 표현하는 내부 클래스
    class State
    {
        friend class Memento;
        friend class MinwooPlayer;
    private:
        int gemType = 0;            // 한 붓에 그려지는 보석의 종류
        bool diagonal = false;      // 대각선 이동을 했는지 유무
        bool finish = false;        // 한 붓 그리기 탐색이 끝났는지 유무
        SearchDirection::Enum sd;   // 다음 보석으로 이동할 수 있는 방향을 저장
        vector<Point> resultVect;   // 한 붓에 포함되는 보석&폭탄: size()로 한붓 기본 점수 계산
        vector<Point> bombVect;     // 한 붓에 포함되는 폭탄: resultVect의 일부임(Validate에서 생성)
        set<Point> burstSet;        // 폭탄의 폭발로 터지는 보석들: 1개당 1점

    public:
        void Copy(const shared_ptr<State> src)
        {
            gemType = src->gemType;
            diagonal = src->diagonal;
            finish = src->finish;
            sd = src->sd;
            resultVect = src->resultVect;
            bombVect = src->bombVect;
            burstSet = src->burstSet;
        }
    };

    // 상태를 관리하는 내부 클래스
    class Memento
    {
    private:
        shared_ptr<State> state;
    public:
        Memento() { state = make_shared<State>(); }
        Memento(const shared_ptr<State> s) { state = make_shared<State>(); state->Copy(s); }
        inline void SetState(const shared_ptr<State> s) { state->Copy(s); }
        inline shared_ptr<State> GetState() const { return state; }
    };

    // 메멘토 저장/복원 메소드
    inline shared_ptr<Memento> CreateMemento() const { return make_shared<Memento>(state); }
    void SetMemento(const shared_ptr<Memento> src) { state->Copy(src->GetState()); }

    // 현재 상태
    shared_ptr<State> state;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 게임 기능 관련 메소드
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Validate();
    void FinishGame();
    int CalculateScore();
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 보조 기능 메소드
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // board의 index와 Point의 관계를 서로 바꿈
    inline Point IdxToPoint(int idx) const { return Point{ idx%width, idx / width }; }
    inline int PointToIdx(int x, int y) const { return y*width + x; }
    inline int PointToIdx(Point p) const { return PointToIdx(p.x, p.y); }

    // (x, y)에 해당하는 gem 종류 반환, (0, 0)부터 시작
    inline int GetGemType(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return -1;
        return board[PointToIdx(x, y)];
    }
    inline int GetGemType(Point p) const { return GetGemType(p.x, p.y); }

    // p가 gemType과 같거나 폭탄이면 true
    inline bool IsSameGemOrBomb(int gemType, int x, int y) const {
        return (GetGemType(x, y) > 100 || GetGemType(x, y) == gemType) ? true : false;
    }
    inline bool IsSameGemOrBomb(int gemType, Point p) const { return IsSameGemOrBomb(gemType, p.x, p.y); }

    // Point의 상하좌우, 대각선 표현
    inline Point Left(Point p) { return Point{ p.x - 1, p.y }; }
    inline Point Right(Point p) { return Point{ p.x + 1, p.y }; }
    inline Point Up(Point p) { return Point{ p.x, p.y + 1 }; }
    inline Point Down(Point p) { return Point{ p.x, p.y - 1 }; }
    inline Point LeftUp(Point p) { return Point{ p.x - 1, p.y + 1 }; }
    inline Point LeftDown(Point p) { return Point{ p.x - 1, p.y - 1 }; }
    inline Point RightUp(Point p) { return Point{ p.x + 1, p.y + 1 }; }
    inline Point RightDown(Point p) { return Point{ p.x + 1, p.y - 1 }; }
    inline Point MovePoint(Point p, SearchDirection::Enum sd)
    {
        switch (sd)
        {
        case SearchDirection::Left:
            return Left(p);
        case SearchDirection::Right:
            return Right(p);
        case SearchDirection::Up:
            return Up(p);
        case SearchDirection::Down:
            return Down(p);
        case SearchDirection::LeftUp:
            return LeftUp(p);
        case SearchDirection::LeftDown:
            return LeftDown(p);
        case SearchDirection::RightUp:
            return RightUp(p);
        case SearchDirection::RightDown:
            return RightDown(p);
        }
        return p;
    }

};

