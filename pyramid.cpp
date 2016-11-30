#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/****************************************************************************/
#define LAYERS    7
#define WIDTH     7
#define STOCK_LEN 24
#define STOCK_MAX_ROUND 3

/****************************************************************************/
class Pos{
public:
    int layer;
    int x;
    Pos():layer(0),x(0){}
    Pos(int layer_, int x_):layer(layer_),x(x_){}
    
    static const Pos stock1;
    static const Pos stock2;
    static const Pos nopos;
    bool isOnBoard(){return layer>=1;}
    bool operator==(const Pos &p2)const{return x==p2.x && layer==p2.layer;}
    bool operator!=(const Pos &p2)const{return x!=p2.x || layer!=p2.layer;}
    void print(){if(layer==-1){printf("st1");}else if(layer==-2){printf("st2");}else{printf("%d%d",layer,x);}}
};
const Pos Pos::stock1(-1,-1); //右
const Pos Pos::stock2(-2,-2); //左
const Pos Pos::nopos(0,0);

/****************************************************************************/
class Move{
public:
    Move(): isDraw(false){}
    Move(const Pos &p1_, const Pos &p2_):p1(p1_),p2(p2_),isDraw(false){}
    Pos p1,p2;
    bool isDraw;
};

/****************************************************************************/
class Board{
public:
    enum{
        card_invalid=-1,
        card_empty=0,
    };

    int tableau[LAYERS+2][WIDTH+2];  //例：[1][1]の上(画面上は下)に[1][2], [2][2]がかぶってる
    signed char stock[STOCK_LEN+2];  //stock[1]が最初のカード。先頭・末尾にemptyをつける。
    int stock_len;
    int stock_nowpos;
    int stock_round;
    int pile_card;
    int tesuu;
    
    typedef struct _history{
        Move m;
        int p1_prev;
        int p2_prev;
        signed char stock_prev[STOCK_LEN+2];
        int nowpos_prev;
        int round_prev;
    }History;
    History history[26+24+24+24];
    
    void init();
    void init(int argc, const char *argv[]);
    void print();
    
    int  getCard(Pos p);
    bool isComplete(){return tableau[1][1]==card_empty;}
    bool isstockend(){return stock_nowpos>=stock_len;}
    bool isroundend(){return stock_round==STOCK_MAX_ROUND-1;}
    bool isstockover(){return isstockend() && isroundend();}
    bool lastMoveIsDraw(){return tesuu>=1 && history[tesuu-1].m.isDraw;}
    bool isExposed(int layer, int x)const{return tableau[layer+1][x]== card_empty && tableau[layer+1][x+1]==card_empty;};
    bool isremovable(int layer, int x)const;
    void search_candidate(Move candidate[16], int *num);
    void search_king(Move candidate[16], int *num);
    
    void record_history(Move m);
    void remove(Move m);
    void remove_stock(int spos);
    void draw();
    void undo();
    
    int  c2i(char c);
    char i2c(int i){return " A234567890JQK*"[i];}
    void setTableau(int layer, int x, char val){tableau[layer][x]=c2i(val);}
    void setTableau_layerall(int layer, const char *);
}board;
/****************************************************************************/
void Board::init()
{
    memset( tableau, 0, sizeof(tableau) );
    memset( stock,   0, sizeof(stock)   );
    stock_len = stock_nowpos = stock_round = pile_card = tesuu = 0;
}
/****************************************************************************/
void Board::init(int argc, const char *argv[])
{
    init();
    if( argc!= 1+LAYERS+1 ){
        fprintf(stderr, "cmd line option num err.\n");
        exit(0);
    }

    //枚数チェック
    int count[13+1]={0};

    //場のデータ格納
    for( int layer=1; layer<=LAYERS; layer++ ){
        if( strlen(argv[layer])!=(size_t)layer){
            fprintf(stderr, "cmd line option length err.\n");
            exit(0);
        }
        for( int x=1; x<=layer; x++){
            tableau[layer][x] = c2i(argv[layer][x-1]);
            count[tableau[layer][x]]++;
        }
    }
    
    //Stockデータ格納
    if(strlen(argv[LAYERS+1])!=STOCK_LEN){
        fprintf(stderr, "cmd line option length err.\n");
        exit(0);
    }
    for( int i=0; i<STOCK_LEN; i++){ //先頭はempty,[1]～[24]に格納
        stock[i+1]=c2i(argv[LAYERS+1][i]);
        count[stock[i+1]]++;
    }
    stock_len = STOCK_LEN;
    
    //カウント数チェック：すべて４枚ずつであること
    for( int i=1; i<=13; i++){
        if( count[i]!=4 ){
            printf("err: card=%d is not 4 cards.\n",i);
            exit(1);
        }
    }
}

/****************************************************************************/
void Board::print()
{
    printf("\n");
    for(int layer=1; layer<=LAYERS; layer++ ){
        for( int i=0; i<LAYERS-layer; i++){printf(" ");}
        for( int x=1; x<=layer; x++ ){
            printf("%c ", i2c(tableau[layer][x]));
        }
        printf("\n");
    }

    printf("stock: round=%d\n", stock_round);
    for( int i=stock_len+1; i>=0; i-- ){
        printf("%c", i2c(stock[i]));
    }
    printf("\n");
    for( int i=0; i<stock_len-stock_nowpos; i++){printf(" ");}printf("^^\n");
    assert(stock_nowpos==0 || stock_nowpos<=stock_len);
    
    printf("history: ");
    for( int i=0; i<tesuu; i++){
        if( history[i].m.isDraw ){
            printf("draw");
        }else{
            history[i].m.p1.print();
            printf("-");
            history[i].m.p2.print();
        }
        printf(":");
    }
    printf("\n");
}
/****************************************************************************/
int  Board::getCard(Pos p)
{
    if( p==Pos::stock1 ){
        return stock[stock_nowpos];
    }else if( p==Pos::stock2 ){
        return stock[stock_nowpos+1];
    }else if( p==Pos::nopos ){
        return 0;
    }else{
        return tableau[p.layer][p.x];
    }
}

/****************************************************************************/
void Board::search_candidate(Move candidate[16], int *num)
{
    //exposedなcardを列挙
    Pos exposed[7];
    int expo_num = 0;
    for( int x=1; x<=WIDTH; x++){
        for( int layer=LAYERS; layer>=1; layer--){
            if( tableau[layer][x]==card_empty ){continue;}
            if( isExposed(layer,x) ){
                assert(expo_num<=6);
                exposed[expo_num] = Pos(layer,x);
                expo_num++;
            }
            //empty以外なので、どっちにしろもう上を調べる必要なし
            break;
        }
    }

#ifdef DEBUG
    printf("exposed card: ");
    for( int i=0; i<expo_num; i++){
        printf("%d%d:", exposed[i].layer, exposed[i].x);
    }
    printf("\n");
#endif
    
    *num = 0;

    //足して13になる組を検索
    //ただし、draw直後の場合は禁則とする。(draw前にできているはず)
    if( ! lastMoveIsDraw() ){
        for( int i=0; i<expo_num; i++ ){
            for( int j=i+1; j<expo_num; j++ ){
                if( tableau[exposed[i].layer][exposed[i].x]
                  + tableau[exposed[j].layer][exposed[j].x] == 13 ){
                    assert(*num<16);
                    candidate[*num] = Move(exposed[i],exposed[j]);
                    (*num)++;
                }
            }
        }

        //ストックエリアとの組み合わせを検索
        //ストック右との組み合わせは、draw直後禁足(draw前にできているはず)
        for( int i=0; i<expo_num; i++ ){
            if( tableau[exposed[i].layer][exposed[i].x]
                    +stock[stock_nowpos] == 13 ){
                candidate[*num] = Move(exposed[i], Pos::stock1);
                (*num)++;
            }
        }
    }

    for( int i=0; i<expo_num; i++ ){
        if( tableau[exposed[i].layer][exposed[i].x]
                +stock[stock_nowpos+1] == 13 ){
            candidate[*num] = Move(exposed[i], Pos::stock2);
            (*num)++;
        }
    }
    
    //ストックエリア2枚のちぇっく
    if( stock[stock_nowpos] + stock[stock_nowpos+1] == 13 ){
        candidate[*num] = Move(Pos::stock1, Pos::stock2);
        (*num)++;
    }

#ifdef DEBUG
    printf("candidate: ");
    for( int i=0; i<*num; i++){
        printf("%d%dx%d%d:", candidate[i].p1.layer,
                             candidate[i].p1.x,
                             candidate[i].p2.layer,
                             candidate[i].p2.x);
    }
#endif

}
/****************************************************************************/
void Board::search_king(Move candidate[16], int *num)
{
    //search_candidate と同じI/Fだが、１こ見つけたら終了する。
    *num = 0;
    for( int x=1; x<=WIDTH; x++){
        for( int layer=LAYERS; layer>=1; layer--){
            if( tableau[layer][x]==card_empty ){continue;}
            if( isExposed(layer,x) ){
                if( tableau[layer][x]==13 ){
                    //発見
                    candidate[0] = Move(Pos(layer,x),Pos(0,0));
                    *num=1;
                    return;
                }
            }
            //empty以外なので、どっちにしろもう上を調べる必要なし
            break;
        }
    }

    if( stock[stock_nowpos]==13 ){
        candidate[0] = Move(Pos::stock1,Pos(0,0));
        *num=1;
        return;
    }

    if( stock[stock_nowpos+1]==13 ){
        candidate[0] = Move(Pos::stock2,Pos(0,0));
        *num=1;
        return;
    }
    //*num = 0 のままreturn;
}

/****************************************************************************/
void Board::record_history(Move m)
{
    history[tesuu].m = m;
    memcpy( history[tesuu].stock_prev, stock, sizeof(stock) );
    history[tesuu].p1_prev = getCard(m.p1);
    history[tesuu].p2_prev = getCard(m.p2);
    history[tesuu].nowpos_prev = stock_nowpos;
    history[tesuu].round_prev = stock_round;
    tesuu++;
}

/****************************************************************************/
void Board::remove(Move m)
{
    assert( m.p1 != m.p2  );

    record_history(m);
    
    assert(getCard(m.p1)!=card_empty);
    //TODO: 足して13になるチェック

    
    //先にstock左かチェック(右を先にやるとずれてうまくいかない)
    if( m.p1 == Pos::stock2 ){
        assert(stock[stock_nowpos+1] != card_empty);
        remove_stock(stock_nowpos+1);
    }else if( m.p2 == Pos::stock2 ){
        assert(stock[stock_nowpos+1] != card_empty);
        remove_stock(stock_nowpos+1);
    }
    
    //右
    if( m.p1 == Pos::stock1 ){
        assert(stock[stock_nowpos] != card_empty);
        remove_stock(stock_nowpos);
    }else if( m.p2 == Pos::stock1 ){
        assert(stock[stock_nowpos] != card_empty);
        remove_stock(stock_nowpos);
    }
    
    if( m.p1.isOnBoard() ){
        assert(tableau[m.p1.layer][m.p1.x]!=card_empty);
        tableau[m.p1.layer][m.p1.x] = card_empty;
    }
    
    if( m.p2.isOnBoard() ){
        assert(tableau[m.p2.layer][m.p2.x]!=card_empty);
        tableau[m.p2.layer][m.p2.x] = card_empty;
    }
    //print();
}
/****************************************************************************/
void Board::remove_stock(int spos)
{
    assert(stock[spos] != card_empty);
    //printf("removing %d, pos=%d, stock_nowpos=%d\n",
    //            stock[spos], spos, stock_nowpos);
    memmove(&stock[spos], &stock[spos+1], sizeof(stock)-spos-1);
    stock_len--;

    if( spos == stock_nowpos ){
        if( stock_nowpos>0 ){ stock_nowpos--; }
    }else if( spos == stock_nowpos+1 ){
        //do nothing
    }else{
        assert(0);
    }
}

/****************************************************************************/
void Board::draw()
{
    Move m;
    m.isDraw = true;
    record_history(m);

    if( isstockend() ){
        assert(!isroundend());
        stock_round++;
        stock_nowpos=0;
    }else{
        stock_nowpos++;
    }
}
/****************************************************************************/
void Board::undo()
{
    //puts("######undo!#######");

    History h = history[tesuu-1];
    
    if( h.m.p1.isOnBoard() ){
        assert(tableau[h.m.p1.layer][h.m.p1.x]==card_empty);
        tableau[h.m.p1.layer][h.m.p1.x] = h.p1_prev;
    }
    if( h.m.p2.isOnBoard() ){
        assert(tableau[h.m.p2.layer][h.m.p2.x]==card_empty);
        tableau[h.m.p2.layer][h.m.p2.x] = h.p2_prev;
    }
    memcpy(stock, h.stock_prev, sizeof(stock) );
    stock_len = strlen((char*)&stock[1]);
    stock_nowpos = h.nowpos_prev;
    stock_round  = h.round_prev;
    
    tesuu --;
}

/****************************************************************************/
int Board::c2i(char c)
{
    c = toupper(c);
    const char table[] = "A234567890JQK";
    const char *pos = strchr( table, c);
    if( pos ){return pos-table+1;}
    if( c=='1' ){return 1;}
    if( c==' ' ){return card_empty;}
    return card_invalid;
}
/****************************************************************************/
void Board::setTableau_layerall(int layer, const char *data)
{
    assert(strlen(data)>=(size_t)layer);
    for(int i=0; i<layer+1; i++){
        tableau[layer][i] = c2i(data[i]);
    }
}

/****************************************************************************/
void read_cardseq(const char prompt[], int layer, int length)
{
    static char  buf[80];
    for(;;){
        printf("Input layer-3 sequence > ");
        fgets(buf, sizeof(buf), stdin);
        
        //length check
        if( strlen(buf)!=WIDTH+1 ){ continue; } //+1 means '\n'
        if( buf[WIDTH]!='\n' ){continue;}
        buf[WIDTH]='\0';
        
        //to Upper
        int i;
        for(int i=0; i<WIDTH; i++){
            buf[i] = toupper(buf[i]);
        }
        
        //check 数字 or AJQK
        for( i=0; i<WIDTH; i++){
            if( strchr("1234567890JQKA", buf[i])==NULL){
                break;
            }
        }
        if(i!=WIDTH){
        	continue;  //check NG
        }
        break; //input OK
    }
    board.setTableau_layerall(layer, buf);
}

/****************************************************************************/
void usage()
{
    exit(0);
}

/****************************************************************************/
void solve(Board &board)
{
    if( board.isComplete() ){
        board.print();
        printf("Congraturation!!\n");
        exit(0);
    }

    //board.print();

    Move candidate[16];
    int  num;
    
    //まずは手が１択状態のチェック
    if( board.search_king(candidate, &num), num!=0 ){ //kingのサーチ。
        //１つでもみつかれば、１択として進める。
        //search_kingは１個見つけたらサーチ終了する
            board.remove(candidate[0]);
            solve(board);  //もし関数から返ってきたら、NGだったということ
            board.undo();
    }else{
        //kingが無い
        board.search_candidate(candidate, &num);
        for( int i=0; i<num; i++){
            board.remove(candidate[i]);
            solve(board);  //もし関数から返ってきたら、NGだったということ
            board.undo();
        }
        //forを抜けてしまった＝removeする手が全NG or removeできない
        if( !board.isstockover() ){
            board.draw();
            solve(board);  //もし関数から返ってきたら、NGだったということ
            board.undo();
        }
    }
    
    //ここまで来たら、すべての手がNG,どんずまり。
}

/****************************************************************************/
int test();
int main(int argc, const char *argv[])
{
#ifdef TEST
    if(argc>=2 && strcmp(argv[1],"--test")==0 ){
        return test();
    }
#endif

    Board board;

    board.init(argc,argv);
    board.print();
    solve(board);
    return 0;
}
/****************************************************************************/
#ifdef TEST
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

class FunctionTest : public CPPUNIT_NS::TestFixture{
    //テストクラス
    CPPUNIT_TEST_SUITE(FunctionTest);//登録のスタート
    CPPUNIT_TEST(test_test);
    CPPUNIT_TEST_SUITE_END();//登録の終了

protected:
    Board *pBoard;
    void test_test();

public:
    void setUp();
    void testDown();
};

/****************************************************************************/
CPPUNIT_TEST_SUITE_REGISTRATION(FunctionTest);

//テスト起動時に実行
void FunctionTest::setUp(){
    pBoard = new Board();
}

//テスト終了時に実行
void FunctionTest::testDown(){
    delete pBoard;
}
/****************************************************************************/
void FunctionTest::test_test()
{
    const char *testdata[]={"hoge", "7","3j","620","280j","4j0ak","7k5q58","12k8462",
                     "4q17q0jkq716593394863599"};
    pBoard->init(9, testdata);
    pBoard->print();

    pBoard->remove(Move(Pos(7,3),Pos(0,0)));  //remove king
    CPPUNIT_ASSERT_EQUAL(0, pBoard->tableau[7][3]);

    pBoard->print();
    pBoard->draw(); pBoard->draw(); pBoard->draw(); pBoard->draw();
    pBoard->draw(); pBoard->draw(); pBoard->draw(); pBoard->draw();
    pBoard->draw(); pBoard->draw(); pBoard->draw(); pBoard->draw();
    pBoard->draw(); pBoard->draw(); pBoard->draw(); pBoard->draw();
    pBoard->draw(); pBoard->draw(); pBoard->draw(); pBoard->draw();
    pBoard->draw(); pBoard->draw(); pBoard->draw(); pBoard->draw();
    pBoard->draw(); pBoard->draw();
    pBoard->print();

    pBoard->remove(Move(Pos(7,1),Pos::stock2));  //remove A-Q
    pBoard->print();
    CPPUNIT_ASSERT_EQUAL(0, pBoard->tableau[7][1]);
    CPPUNIT_ASSERT_EQUAL(23, pBoard->stock_len);
    
    pBoard->undo();
    pBoard->print();
    CPPUNIT_ASSERT_EQUAL(1, pBoard->tableau[7][1]);
    CPPUNIT_ASSERT_EQUAL(24, pBoard->stock_len);

    pBoard->undo();
    pBoard->undo();
    pBoard->print();
    CPPUNIT_ASSERT_EQUAL(0, pBoard->stock_round);
}
/****************************************************************************/
int test()
{
    CPPUNIT_NS::TestResult controller;

    //結果収集
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);

    //途中結果の収集
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);

    //テストを走らせる。テストを入れて走る
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
    runner.run(controller);

    //結果を標準出力にする。
    CPPUNIT_NS::CompilerOutputter outputter(&result,CPPUNIT_NS::stdCOut());
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}
#endif //TEST
