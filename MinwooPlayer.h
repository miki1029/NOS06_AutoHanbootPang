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
    // ���� ����
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Memento;
    // �� �� �׸��� ���¸� ǥ���ϴ� ���� Ŭ����
    class State
    {
        friend class Memento;
        friend class MinwooPlayer;
    private:
        int gemType = 0;            // �� �׿� �׷����� ������ ����
        bool diagonal = false;      // �밢�� �̵��� �ߴ��� ����
        bool finish = false;        // �� �� �׸��� Ž���� �������� ����
        SearchDirection::Enum sd;   // ���� �������� �̵��� �� �ִ� ������ ����
        vector<Point> resultVect;   // �� �׿� ���ԵǴ� ����&��ź: size()�� �Ѻ� �⺻ ���� ���
        vector<Point> bombVect;     // �� �׿� ���ԵǴ� ��ź: resultVect�� �Ϻ���(Validate���� ����)
        set<Point> burstSet;        // ��ź�� ���߷� ������ ������: 1���� 1��

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

    // ���¸� �����ϴ� ���� Ŭ����
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

    // �޸��� ����/���� �޼ҵ�
    inline shared_ptr<Memento> CreateMemento() const { return make_shared<Memento>(state); }
    void SetMemento(const shared_ptr<Memento> src) { state->Copy(src->GetState()); }

    // ���� ����
    shared_ptr<State> state;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ���� ��� ���� �޼ҵ�
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Validate();
    void FinishGame();
    int CalculateScore();
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ���� ��� �޼ҵ�
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // board�� index�� Point�� ���踦 ���� �ٲ�
    inline Point IdxToPoint(int idx) const { return Point{ idx%width, idx / width }; }
    inline int PointToIdx(int x, int y) const { return y*width + x; }
    inline int PointToIdx(Point p) const { return PointToIdx(p.x, p.y); }

    // (x, y)�� �ش��ϴ� gem ���� ��ȯ, (0, 0)���� ����
    inline int GetGemType(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return -1;
        return board[PointToIdx(x, y)];
    }
    inline int GetGemType(Point p) const { return GetGemType(p.x, p.y); }

    // p�� gemType�� ���ų� ��ź�̸� true
    inline bool IsSameGemOrBomb(int gemType, int x, int y) const {
        return (GetGemType(x, y) > 100 || GetGemType(x, y) == gemType) ? true : false;
    }
    inline bool IsSameGemOrBomb(int gemType, Point p) const { return IsSameGemOrBomb(gemType, p.x, p.y); }

    // Point�� �����¿�, �밢�� ǥ��
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

