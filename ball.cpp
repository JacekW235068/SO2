#include <mutex>
#include <thread>
#include <condition_variable>
#include <list>
#ifndef BALL
#define BALL
using namespace std;

class Ball
{
        
    public:
    //constructors
        Ball(const Ball& b)
        : xupdate(b.xupdate), yupdate(b.yupdate), posx(b.posx), posy(b.posy), speedx(b.speedx), speedy(b.speedy),show(b.show),  balls(b.balls)
        {
            go=true;
            odbited=false;
        }
        Ball(float x,float y,float sx,float sy,int xup, int yup, condition_variable &s, list<Ball> *b)
        : xupdate(xup), yupdate(yup), posx(x), posy(y), speedx(sx), speedy(sy), show(s), balls(b)
        {
            go = true;
            odbited=false;   
        }
        ~Ball(){
        }
        //methods
        void moveBall(){
            imasleep = false;
            float tx,ty;
            static mutex m;
            
            do{
                if(speedx >=0)
                    tx=((float)(xupdate+1)-posx)/speedx;
                else
                    tx=((float)(xupdate-1)-posx)/speedx;
                if(speedy >=0)
                    ty=((float)(yupdate+1)-posy)/speedy;
                else
                    ty=((float)(yupdate-1)-posy)/speedy;
                //tx>2*ty || (ty>tx && ty<2*tx )
                //-----------------
                if(ty<tx){
                    tx=ty;
                }
                {
                    unique_lock<mutex> lk(m);
                    
                    bool gotosleep=false;
                    posx=posx+(speedx*tx);
                    posy=posy+(speedy*tx);
                    speedy += tx*1.5;
                    bool changedx = xupdate != static_cast<int>(posx);
                    xupdate = static_cast<int>(posx);
                    yupdate = static_cast<int>(posy);
                    if(!odbited){
                    for(Ball &ball : *balls){
                        if (&ball != this && ball.xupdate == this->xupdate){
                            if(ball.imasleep){
                                ball.cv.notify_one();
                                continue;
                            }
                            ball.speedx = -ball.speedx;
                            ball.odbited = true;
                            gotosleep=true;
                            
                        }
                    }
                    if(gotosleep){
                        imasleep = true;
                        cv.wait(lk);
                        imasleep=false;
                        odbited=true;
                    }
                    }else 
                    if(changedx){
                        odbited=false;
                        }
                }
                this_thread::sleep_for(chrono::milliseconds((int)(200*tx)));
                show.notify_one();
                if(xupdate >=59)
                    speedx=-speedx;
                else if(xupdate <=1){
                    speedx=abs(speedx);
                }
                if (yupdate >=19)
                    speedy=-speedy*0.9;
                else if(yupdate <=1){
                    speedy=abs(speedy);
                }
            
            }while(this->go);            
        }
        //properties
        bool odbited;
        condition_variable &show;
        list<Ball> *balls;
        bool imasleep;
        int xupdate;
        int yupdate;
        bool go;
        float speedx;
        condition_variable cv;
    private:
        float posx;
        float posy;
        float speedy;
};

#endif