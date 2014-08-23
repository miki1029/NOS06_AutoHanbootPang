#pragma once
class IPlayer
{
public:
    virtual void PrepareGame(
                    int width, int height,
                    int gemTypeCount, int totalTurnCount) = 0;
    virtual int Play(int phase, int* board, int* result) = 0;
};