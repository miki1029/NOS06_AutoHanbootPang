#include "MinwooPlayer.h"


MinwooPlayer::MinwooPlayer()
{
}


MinwooPlayer::~MinwooPlayer()
{
}


void MinwooPlayer::PrepareGame(
    int width, int height,
    int gemTypeCount, int totalTurnCount)
{
    this->width = width;
    this->height = height;
    this->gemTypeCount = gemTypeCount;
    this->totalTurnCount = totalTurnCount;
    this->boardSize = width * height;
}


int MinwooPlayer::Play(int phase, int* board, int* result)
{
    this->board = board;
    shared_ptr<Memento> bestResult = make_shared<Memento>();
    int bestScore = 0;
    queue<shared_ptr<Memento>> mementoQ;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            state = make_shared<State>();

            // 탐색 출발점
            Point startPoint = Point{ x, y };
            state->resultVect.push_back(startPoint);
            state->gemType = GetGemType(startPoint);
            Point curPoint = startPoint;

            // 좌우 또는 상하로 같은 색이거나 폭탄이면 탐색 출발점으로 삼을 필요가 없음
            if (state->gemType <= gemTypeCount &&
                (IsSameGemOrBomb(state->gemType, Left(startPoint)) && IsSameGemOrBomb(state->gemType, Right(startPoint))) ||
                (IsSameGemOrBomb(state->gemType, Up(startPoint)) && IsSameGemOrBomb(state->gemType, Down(startPoint))))
                continue;

            // 출발점이 폭탄인 경우

            // 탐색 시작
            while(true)
            {
                stack<SearchDirection::Enum> joinableStack;
                // 8방향 확인
                if (!state->finish)
                {
                    vector<SearchDirection::Enum> sdVect;   // 4방향 또는 8방향을 저장
                    sdVect.push_back(SearchDirection::Left);
                    sdVect.push_back(SearchDirection::Right);
                    sdVect.push_back(SearchDirection::Up);
                    sdVect.push_back(SearchDirection::Down);
                    if (!state->diagonal) {    // 아직 대각선 이동을 하지 않은 경우 8방향
                        sdVect.push_back(SearchDirection::LeftUp);
                        sdVect.push_back(SearchDirection::LeftDown);
                        sdVect.push_back(SearchDirection::RightUp);
                        sdVect.push_back(SearchDirection::RightDown);
                    }
                    // 각 방향에 대해 이을 수 있는 점인지 확인
                    for each (SearchDirection::Enum direction in sdVect)
                    {
                        Point checkPoint = MovePoint(curPoint, direction);
                        if (IsSameGemOrBomb(state->gemType, checkPoint))
                        {
                            vector<Point>::iterator itr =
                                find(state->resultVect.begin(), state->resultVect.end(), checkPoint);
                            // 한 붓에 포함되지 않았던 점이면 검사영역에 추가
                            if (itr == state->resultVect.end())
                                joinableStack.push(direction);
                        }
                    }
                }
                // 이을 수 있는 경우가 없다면 현재까지의 최적 결과와 비교
                if (joinableStack.empty())
                {
                    if (Validate())     // 폭탄이 포함되면 유효한 한붓인지 확인 필요
                    {
                        FinishGame();   // 폭탄을 터뜨려 터지는 보석 계산
                        int score = CalculateScore();   // 점수 계산
                        cout << score << "," << bestScore << endl;
                        if (score > bestScore)
                        {
                            bestResult = CreateMemento();
                            bestScore = score;
                        }
                    }
                    // 큐에 더이상 남은 상태가 없으면 반복문 종료
                    if (mementoQ.empty()) break;
                }
                // 이을 수 있는 경우가 존재한다면 큐에 추가
                else while (!joinableStack.empty())
                {
                    state->sd = joinableStack.top();
                    joinableStack.pop();
                    mementoQ.push(CreateMemento());
                }
                // 큐에서 하나를 꺼냄
                SetMemento(mementoQ.front());
                mementoQ.pop();

                // 한붓 이동
                curPoint = MovePoint(state->resultVect.back(), state->sd);
                switch (state->sd)
                {
                case SearchDirection::LeftUp:
                case SearchDirection::LeftDown:
                case SearchDirection::RightUp:
                case SearchDirection::RightDown:
                    state->diagonal = true;
                }
                state->resultVect.push_back(curPoint);
                // 8회 이동이 끝났으면 종료상태
                if (state->resultVect.size() >= 8)
                {
                    state->finish = true;
                }
            }
        }
    }
    // 최적 결과를 가져옴
    SetMemento(bestResult);
    int numOfResult = state->resultVect.size();
    for (int i = 0; i < numOfResult; i++)
    {
        result[i] = PointToIdx(state->resultVect[i]);
    }
    return numOfResult;
}


bool MinwooPlayer::Validate()
{
    // bombVect 초기화
    for each (Point checkPoint in state->resultVect)
    {
        if (GetGemType(checkPoint) > 100)
        {
            state->bombVect.push_back(checkPoint);
        }
    }
    // 폭탄이 없으면 문제 없음
    if (state->bombVect.empty()) return true;
    // 최소 비 폭탄 보석 2개 포함해야 함
    else if (state->resultVect.size() - state->bombVect.size() >= 2) return true;
    // 그렇지 않으면 유효하지 않음
    else
    {
        state->bombVect.clear();
        return false;
    }
}


void MinwooPlayer::FinishGame()
{
    state->finish = true;
    queue<Point> bombQ;         // resultVect에서 폭탄이 있는 점
    // 한 붓에 포함된 폭탄 추가
    for each (Point bombPoint in state->bombVect)
    {
        bombQ.push(bombPoint);
    }
    // 폭탄 순회하며 폭발
    while (!bombQ.empty())
    {
        Point bombPoint = bombQ.front();        // 폭탄
        bombQ.pop();
        vector<SearchDirection::Enum> sdVect;   // 폭발 방향을 저장(폭탄1, 폭탄2)
        vector<Point> gemVect;                  // 같은 보석류를 저장하는 벡터(폭탄3)
        // 폭발
        switch (GetGemType(bombPoint))
        {
        case 102:   // 2 레벨
            sdVect.push_back(SearchDirection::LeftUp);
            sdVect.push_back(SearchDirection::LeftDown);
            sdVect.push_back(SearchDirection::RightUp);
            sdVect.push_back(SearchDirection::RightDown);
        case 101:   // 1 레벨, 2 레벨
            sdVect.push_back(SearchDirection::Left);
            sdVect.push_back(SearchDirection::Right);
            sdVect.push_back(SearchDirection::Up);
            sdVect.push_back(SearchDirection::Down);
            // 각 방향으로 폭발
            for each (SearchDirection::Enum direction in sdVect)
            {
                Point burstPoint = MovePoint(bombPoint, direction);
                vector<Point>::iterator itr =
                    find(state->resultVect.begin(), state->resultVect.end(), burstPoint);
                // 한 붓에 포함되지 않았던 점이면 burstSet에 추가
                if (itr == state->resultVect.end())
                    state->burstSet.insert(burstPoint);
                // burstPoint가 폭탄이면 bombQ에 추가
                if (GetGemType(burstPoint) > 100)
                    bombQ.push(burstPoint);
            }
            break;
        case 103:   // 3 레벨
            // Scan
            for (int i = 0; i < boardSize; i++)
            {
                if (board[i] == state->gemType) gemVect.push_back(IdxToPoint(i));
            }
            for each (Point burstPoint in gemVect)
            {
                vector<Point>::iterator itr =
                    find(state->resultVect.begin(), state->resultVect.end(), burstPoint);
                // 한 붓에 포함되지 않았던 점이면 burstSet에 추가
                if (itr == state->resultVect.end())
                    state->burstSet.insert(burstPoint);
            }
            break;
        default:
            break;
        }
    }
}


int MinwooPlayer::CalculateScore() {
    int score = 0;
    // 한 붓 기본점수
    switch (state->resultVect.size())
    {
    case 1: case 2: case 3: case 4:
        score = state->resultVect.size();
        break;
    case 5:
        score = 6;
        break;
    case 6:
        score = 8;
        break;
    case 7:
        score = 10;
        break;
    case 8:
        score = 13;
        break;
    default:
        break;
    }
    // 폭탄으로 인한 추가점수
    score += state->burstSet.size();

    return score;
}