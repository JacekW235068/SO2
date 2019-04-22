#include <thread>
#include <mutex>
#include <chrono>
#include <list>
#include <time.h>
#include <stdlib.h>
 #include <ncurses.h>
#include "ball.cpp"


mutex read;
using namespace std;
bool go = true;

void testThread(int &l, mutex &m){
    m.lock();
    for(int i=0; i < 100000; i++){
        l++;
    }
}




void Refresher(list<Ball> &ballz, condition_variable &showLock){
    list<int> oldx;
    list<int> oldy;
    mutex m;
    unique_lock<mutex> lk(m);
    int x,y;
    bool event=false;
    WINDOW *win;
    noecho();
    cbreak();
    initscr();
    win = newwin(21, 61, 0,0);
    refresh();
    curs_set(0);
    box(win, 0,0);
    auto xIt = oldx.begin();
    auto yIt = oldy.begin();
    
    while(go){
        showLock.wait(lk);
        if(oldx.size()!= ballz.size()){
            oldx.push_back(1);
            oldy.push_back(1);
        }
        xIt = oldx.begin();
        yIt = oldy.begin();
        for(Ball &b: ballz){
            x = b.xupdate;
            y = b.yupdate;
            if(x != *xIt || y != *yIt){
                mvwaddch(win,*yIt,*xIt,' ');
                mvwaddch(win,y,x,'o');
                *xIt = x;
                *yIt = y;
            }
            xIt++;
            yIt++;
        }
        wrefresh(win);
        // this_thread::sleep_for(chrono::milliseconds(50));             
    }
    endwin();
}

void awaitExit(){
 do{
    char i;
    i = getch();
    if(i == 'e')
        go=false;
}while(go);
}


int main(){
    mutex ShowLock; 
    
    list<Ball> balls;
    list<thread> threads;
    condition_variable sh;
    thread t1(Refresher,ref(balls),ref(sh));
    thread t2(awaitExit);
    int i=0;
    do{
        int r = rand()%5 - 2;
        if(r==0)r--;
        float speed = static_cast <float> (rand()/static_cast<float>(RAND_MAX/4)) + 1.0;
        read.lock();
        balls.push_front(Ball(30.0,10.0, speed*r, speed*(abs(r)-2),30,10, sh, &balls));
        threads.push_back(thread(&Ball::moveBall, balls.begin()));
        read.unlock();
        this_thread::sleep_for(chrono::milliseconds(10000));
        i++;     
    }while(go);
    t2.join();
    sh.notify_all();
    t1.join();
    for(Ball &b : balls){
        b.go = false;
        b.cv.notify_all();
    }
    for(thread &t : threads){
        t.join();
    }
    return 0;
}