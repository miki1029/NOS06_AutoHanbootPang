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

            // Ž�� �����
            Point startPoint = Point{ x, y };
            state->resultVect.push_back(startPoint);
            state->gemType = GetGemType(startPoint);
            Point curPoint = startPoint;

            // �¿� �Ǵ� ���Ϸ� ���� ���̰ų� ��ź�̸� Ž�� ��������� ���� �ʿ䰡 ����
            if (state->gemType <= gemTypeCount &&
                (IsSameGemOrBomb(state->gemType, Left(startPoint)) && IsSameGemOrBomb(state->gemType, Right(startPoint))) ||
                (IsSameGemOrBomb(state->gemType, Up(startPoint)) && IsSameGemOrBomb(state->gemType, Down(startPoint))))
                continue;

            // ������� ��ź�� ���

            // Ž�� ����
            while(true)
            {
                stack<SearchDirection::Enum> joinableStack;
                // 8���� Ȯ��
                if (!state->finish)
                {
                    vector<SearchDirection::Enum> sdVect;   // 4���� �Ǵ� 8������ ����
                    sdVect.push_back(SearchDirection::Left);
                    sdVect.push_back(SearchDirection::Right);
                    sdVect.push_back(SearchDirection::Up);
                    sdVect.push_back(SearchDirection::Down);
                    if (!state->diagonal) {    // ���� �밢�� �̵��� ���� ���� ��� 8����
                        sdVect.push_back(SearchDirection::LeftUp);
                        sdVect.push_back(SearchDirection::LeftDown);
                        sdVect.push_back(SearchDirection::RightUp);
                        sdVect.push_back(SearchDirection::RightDown);
                    }
                    // �� ���⿡ ���� ���� �� �ִ� ������ Ȯ��
                    for each (SearchDirection::Enum direction in sdVect)
                    {
                        Point checkPoint = MovePoint(curPoint, direction);
                        if (IsSameGemOrBomb(state->gemType, checkPoint))
                        {
                            vector<Point>::iterator itr =
                                find(state->resultVect.begin(), state->resultVect.end(), checkPoint);
                            // �� �׿� ���Ե��� �ʾҴ� ���̸� �˻翵���� �߰�
                            if (itr == state->resultVect.end())
                                joinableStack.push(direction);
                        }
                    }
                }
                // ���� �� �ִ� ��찡 ���ٸ� ��������� ���� ����� ��
                if (joinableStack.empty())
                {
                    if (Validate())     // ��ź�� ���ԵǸ� ��ȿ�� �Ѻ����� Ȯ�� �ʿ�
                    {
                        FinishGame();   // ��ź�� �Ͷ߷� ������ ���� ���
                        int score = CalculateScore();   // ���� ���
                        cout << score << "," << bestScore << endl;
                        if (score > bestScore)
                        {
                            bestResult = CreateMemento();
                            bestScore = score;
                        }
                    }
                    // ť�� ���̻� ���� ���°� ������ �ݺ��� ����
                    if (mementoQ.empty()) break;
                }
                // ���� �� �ִ� ��찡 �����Ѵٸ� ť�� �߰�
                else while (!joinableStack.empty())
                {
                    state->sd = joinableStack.top();
                    joinableStack.pop();
                    mementoQ.push(CreateMemento());
                }
                // ť���� �ϳ��� ����
                SetMemento(mementoQ.front());
                mementoQ.pop();

                // �Ѻ� �̵�
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
                // 8ȸ �̵��� �������� �������
                if (state->resultVect.size() >= 8)
                {
                    state->finish = true;
                }
            }
        }
    }
    // ���� ����� ������
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
    // bombVect �ʱ�ȭ
    for each (Point checkPoint in state->resultVect)
    {
        if (GetGemType(checkPoint) > 100)
        {
            state->bombVect.push_back(checkPoint);
        }
    }
    // ��ź�� ������ ���� ����
    if (state->bombVect.empty()) return true;
    // �ּ� �� ��ź ���� 2�� �����ؾ� ��
    else if (state->resultVect.size() - state->bombVect.size() >= 2) return true;
    // �׷��� ������ ��ȿ���� ����
    else
    {
        state->bombVect.clear();
        return false;
    }
}


void MinwooPlayer::FinishGame()
{
    state->finish = true;
    queue<Point> bombQ;         // resultVect���� ��ź�� �ִ� ��
    // �� �׿� ���Ե� ��ź �߰�
    for each (Point bombPoint in state->bombVect)
    {
        bombQ.push(bombPoint);
    }
    // ��ź ��ȸ�ϸ� ����
    while (!bombQ.empty())
    {
        Point bombPoint = bombQ.front();        // ��ź
        bombQ.pop();
        vector<SearchDirection::Enum> sdVect;   // ���� ������ ����(��ź1, ��ź2)
        vector<Point> gemVect;                  // ���� �������� �����ϴ� ����(��ź3)
        // ����
        switch (GetGemType(bombPoint))
        {
        case 102:   // 2 ����
            sdVect.push_back(SearchDirection::LeftUp);
            sdVect.push_back(SearchDirection::LeftDown);
            sdVect.push_back(SearchDirection::RightUp);
            sdVect.push_back(SearchDirection::RightDown);
        case 101:   // 1 ����, 2 ����
            sdVect.push_back(SearchDirection::Left);
            sdVect.push_back(SearchDirection::Right);
            sdVect.push_back(SearchDirection::Up);
            sdVect.push_back(SearchDirection::Down);
            // �� �������� ����
            for each (SearchDirection::Enum direction in sdVect)
            {
                Point burstPoint = MovePoint(bombPoint, direction);
                vector<Point>::iterator itr =
                    find(state->resultVect.begin(), state->resultVect.end(), burstPoint);
                // �� �׿� ���Ե��� �ʾҴ� ���̸� burstSet�� �߰�
                if (itr == state->resultVect.end())
                    state->burstSet.insert(burstPoint);
                // burstPoint�� ��ź�̸� bombQ�� �߰�
                if (GetGemType(burstPoint) > 100)
                    bombQ.push(burstPoint);
            }
            break;
        case 103:   // 3 ����
            // Scan
            for (int i = 0; i < boardSize; i++)
            {
                if (board[i] == state->gemType) gemVect.push_back(IdxToPoint(i));
            }
            for each (Point burstPoint in gemVect)
            {
                vector<Point>::iterator itr =
                    find(state->resultVect.begin(), state->resultVect.end(), burstPoint);
                // �� �׿� ���Ե��� �ʾҴ� ���̸� burstSet�� �߰�
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
    // �� �� �⺻����
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
    // ��ź���� ���� �߰�����
    score += state->burstSet.size();

    return score;
}