/*
//        Project: Apollo 21
//        Filename: vehicle.c
//        Author: Ömer Furkan
//        Contact: omer.furkan.uyanik1@gmail.com
//        Date: 12 Kasım 2016 - 15:47
//
//        x86 BUILD
//        gcc vehicle.c -z execstack -m32 -w -fno-stack-protector -o vehicle -lpthread
//        x64 BUILD
//        gcc vehicle.c -z execstack -w -fno-stack-protector -o vehicle -lpthread
//
//    Marsa gönderilecek aracın özelliklerini ve iletişimini sağlayacak işlemlerin
//    bulunduğu kaynak dosyası.

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define CONTROL_DEVICE          "127.0.0.1"
#define COMMUNICATION_CH        0x539
#define VEHICLE_NAME            "Apollo 21"

#define Clear_Screen()          printf("\033[H\033[J")




typedef struct
{
    char   _cVehicleName[0x40];
    int    _iFuel;
    int    _iFuelStatus;
    int    _iLocationX;         
    int    _iLocationY;
    int    _iSpeed;
    float  _fGone;
}Apollo21_t;



typedef struct
{
    int _iDistance;             
    int _iFind[0x2];
    int _iWater;
    int _iSand;
    int _iRock;
    
}Mars_t;

struct _sPthread_Args
{
    int _iX;
    int _iY;
};



Apollo21_t *Apollo21;
Mars_t *Mars;
struct _sPthread_Args *_spArgs;



int _iMap[0x12D][0x12D];
int _iMaxSize = 0x12C;
int _iSocketFD;
struct sockaddr_in _sSCommDev;
char _cRecvCommand[0x400];      
char _cSendData[0x400];         
bool _bIsDepature = false;
bool _bControl = false;         
pthread_t _pThread, _pThread1, _pThread2, _pThread3; 


/*

void _vMarsFill()
{
    int _iI, _iJ;
    for(_iI = 0x0; _iI <= _iMaxSize; ++_iI)
    {
        for(_iJ = 0x0; _iJ <= _iMaxSize; ++_iJ)
        {
            _iMap[_iI][_iJ] = 1;
        }
    }
}

*/


int _iGenerateRandomNumber(int _iMax, int _iPlus)
{
    sleep(0x1);
    time_t _tT;
    srand((unsigned) time(&_tT));
    if(_iPlus != 0x1)
    {
        return rand() % _iMax;
    }
    else
    {
        return (rand() % _iMax) + _iPlus;
    }
}



void *_vUFODetect()
{
    int _iX = _iGenerateRandomNumber(0x12C, 0x0);
    int _iY = _iGenerateRandomNumber(0x12C-0x1, 0x0);

    if(Apollo21->_iLocationX == _iX && Apollo21->_iLocationY == _iY)
    {
        sprintf(_cSendData, "Bilinmeyen Bir Cisimle Karşılaşıldı... Ne Yapmak İstersiniz?\n1- Kaç\n2-İletişime Geç");
        write(_iSocketFD, _cSendData, sizeof(_cSendData));
    }
}



void *_vRoadCalc()
{
    int     _iDirection = 0x1, _iMaxSpeed = 0xFF, _iTime = 0x0, _iCount = 0x0;  
    float   _fGone = 0x0;

    while((float)Mars->_iDistance > Apollo21->_fGone)
    {
        if((float)Mars->_iDistance - Apollo21->_fGone < _fGone)
        {
            _iDirection = 0x0;
        }

        if(_iDirection == 0x1)
        {
            if(Apollo21->_iSpeed < _iMaxSpeed)
            {
                Apollo21->_iSpeed = _iTime * _iTime;
            }
            else if(Apollo21->_iSpeed > _iMaxSpeed)
            {
                Apollo21->_iSpeed = _iMaxSpeed;
                if(_fGone == 0x0 || _iCount == 0x0)
                {
                    _fGone = Apollo21->_fGone;
                    _iCount = _iTime - 0x1;
                }
            }
        }

        if(_iDirection == 0x0)
        {
            if(Apollo21->_iSpeed > 1)
            {
                _iCount--;
                Apollo21->_iSpeed = _iCount * _iCount;
            }
        }

        Apollo21->_fGone += Apollo21->_iSpeed;
        printf("Hız: %dkm/s Yol %0.2fkm\n", Apollo21->_iSpeed, Apollo21->_fGone);

        _iTime++;
        sleep(0x1);
    }

    Apollo21->_iLocationX = _iGenerateRandomNumber(0x12C, 0x0);       
    Apollo21->_iLocationY = _iGenerateRandomNumber(0x12C-0x1, 0x0);   
    _bControl = true;  
    pthread_exit(0x0); 
}


void *_vSendSample()
{

}


void *_vFuelCalc()
{
    int _iX;
    for(_iX = 0x0; _iX <= 0x12C; _iX++)  
    {
        sleep(0x3);  
        if(Apollo21->_iFuel >= 0x0)
        {
            Apollo21->_iFuel = Apollo21->_iFuel - 0x1;  
            if(Apollo21->_iFuel <= 0x64 || Apollo21->_iFuel >= 0x5A) 
            {
                Apollo21->_iFuelStatus = 0x1;  
            }
            else if(Apollo21->_iFuel <= 0x50 || Apollo21->_iFuel >= 0x46) 
            {
                Apollo21->_iFuelStatus = 0x2;
            } 
            else if(Apollo21->_iFuel <= 0x3C || Apollo21->_iFuel >= 0x32) 
            {
                Apollo21->_iFuelStatus = 0x3;  
            }
            else if(Apollo21->_iFuel <= 0x28 || Apollo21->_iFuel >= 0x1E) 
            {
                Apollo21->_iFuelStatus = 0x4;  
            }
            else
            {
                Apollo21->_iFuelStatus = 0x5;
            }
        }
    }
}



void _vSendLocate()
{
    sprintf(_cSendData, "%d", Apollo21->_iLocationX);
    write(_iSocketFD, _cSendData, sizeof(_cSendData));
    sprintf(_cSendData, "%d", Apollo21->_iLocationY);
    write(_iSocketFD, _cSendData, sizeof(_cSendData));
}


void _vss();
void _vPrepareTheEverything()
{

    Clear_Screen(); 
    printf("Araç Hazırlanıyor. Lütfen Bekleyiniz!\n");
    Apollo21 = malloc(sizeof(Apollo21_t));
    Mars = malloc(sizeof(Mars_t)); 
    _spArgs = malloc(sizeof(struct _sPthread_Args));
    Mars->_iDistance = 0xBB8; 
    Mars->_iWater = 0x1;
    Mars->_iSand = 0x2;
    Mars->_iRock = 0x3;
    strncpy(Apollo21->_cVehicleName, VEHICLE_NAME, sizeof(VEHICLE_NAME)); 

    Apollo21->_iFuel = 0x64;
    Apollo21->_iFuelStatus = 0x0;
    Apollo21->_iLocationX = 0x0;
    Apollo21->_iLocationY = 0x0;
    Apollo21->_iSpeed = 0x0;
    Apollo21->_fGone = 0x0;
}



void *_vGoLocate(void *_vLocate)
{
    memset(Mars->_iFind, NULL, sizeof(Mars->_iFind)); 

    struct _sPthread_Args *_spLocate = (struct _sPthread_Args *)_vLocate;
    if(_bControl == true)  
    {
        if(_spLocate->_iX < 0x0 || _spLocate->_iX > 0x12C || _spLocate->_iY < 0x0 || _spLocate->_iY > 0x12C)
        {
            sprintf(_cSendData, "Marsın Dışına Çıkamazsınız!");
            write(_iSocketFD, _cSendData, sizeof(_cSendData));
        }
        else
        {
            while(0x1)
            {
                /* usleep((0x400*0x400)/0x4); */

                if(Apollo21->_iLocationX < _spLocate->_iX)
                {
                    Apollo21->_iLocationX++;
                }
                else if(Apollo21->_iLocationX > _spLocate->_iX)
                {
                    Apollo21->_iLocationX--;
                }

                if(Apollo21->_iLocationY < _spLocate->_iY)
                {
                    Apollo21->_iLocationY++;
                }
                else if(Apollo21->_iLocationY > _spLocate->_iY)
                {
                    Apollo21->_iLocationY--;
                }


                if(Mars->_iFind[0x2] == NULL)
                {
                    _iMap[Apollo21->_iLocationX][Apollo21->_iLocationY] = _iGenerateRandomNumber(0x3, 0x1);
                    if(Mars->_iFind[0x0] != _iMap[Apollo21->_iLocationX][Apollo21->_iLocationY] && Mars->_iFind[0x1] != _iMap[Apollo21->_iLocationX][Apollo21->_iLocationY] && Mars->_iFind[0x2] != _iMap[Apollo21->_iLocationX][Apollo21->_iLocationY])
                    {
                        if(Mars->_iFind[0x0] == NULL)
                        {
                            Mars->_iFind[0x0] = _iMap[Apollo21->_iLocationX][Apollo21->_iLocationY];
                            printf("_iFind[0x0] = %d\n", Mars->_iFind[0x0]);
                        }
                        else if(Mars->_iFind[0x1] == NULL)
                        {
                            Mars->_iFind[0x1] = _iMap[Apollo21->_iLocationX][Apollo21->_iLocationY];
                            printf("_iFind[0x1] = %d\n", Mars->_iFind[0x1]);
                        }
                        else if(Mars->_iFind[0x2] == NULL)
                        {
                            Mars->_iFind[0x2] = _iMap[Apollo21->_iLocationX][Apollo21->_iLocationY];
                            printf("_iFind[0x2] = %d\n", Mars->_iFind[0x2]);
                        }
                    }
                }

                if(Apollo21->_iLocationX == _spLocate->_iX && Apollo21->_iLocationY == _spLocate->_iY)
                {
                    sprintf(_cSendData, "Belirtilen Koordinatlara Vardık.");
                    write(_iSocketFD, _cSendData, sizeof(_cSendData));
                    break;
                }
            }
        }
    }
    else
    {
        sprintf(_cSendData, "Apollo 21 Henüz Marsa Ulaşmadı!");
        write(_iSocketFD, _cSendData, sizeof(_cSendData));
    }
    pthread_exit(0x0);
}





void _vReportStatus()
{
    sprintf(_cSendData, "Araç Adı: %s\nMevcut Yakıt: %d%%\nMevcut Hız: %d\nLokasyon X: %d\nLokasyon Y: %d\nToplam Mesafe: %0.2fkm", Apollo21->_cVehicleName, Apollo21->_iFuel, Apollo21->_iSpeed, Apollo21->_iLocationX, Apollo21->_iLocationY, Apollo21->_fGone);
    write(_iSocketFD, _cSendData, sizeof(_cSendData));
}



void _vDepature()
{
    pthread_create(&_pThread, NULL, &_vFuelCalc, NULL); 
    pthread_create(&_pThread1, NULL, &_vRoadCalc, NULL);
    sprintf(_cSendData, "Apollo 21 Başarılı Bir Şekilde Kalkışa Geçti!");
    write(_iSocketFD, _cSendData, sizeof(_cSendData));
    _bIsDepature = true;
}



int main(int argc, char *argv[])
{
    _vPrepareTheEverything();
    _iSocketFD = socket(AF_INET, SOCK_STREAM, 0x0);
    if(_iSocketFD == -0x1) 
    {
        printf("İletişim Protokolü Oluşturulamadı!\n");
        close(_iSocketFD); 
        return 0x1;
    }

    _sSCommDev.sin_family = AF_INET;
    _sSCommDev.sin_addr.s_addr = inet_addr(CONTROL_DEVICE); 
    _sSCommDev.sin_port = htons(COMMUNICATION_CH);         

    if(connect(_iSocketFD, (struct sockaddr *)&_sSCommDev, sizeof(_sSCommDev)) < 0x0)
    {
        printf("Bağlantıda Çok Fazla Parazit Mevcut. Kontrol Merkezine Bağlanılamadı!\n");
        close(_iSocketFD);
        return 0x1;
    }

    printf("Komuta Merkezine Başarılı Bir Şekilde Bağlanıldı!\n");


    strncpy(_cSendData, Apollo21->_cVehicleName, sizeof(Apollo21->_cVehicleName));
    write(_iSocketFD, _cSendData, sizeof(_cSendData));


    while(0x1)
    {
        if(recv(_iSocketFD, _cRecvCommand, sizeof(_cRecvCommand), 0x0) < 0x0)
        {
            printf("Komuta Merkezinden Gelen Birşeyler Mevcut Fakat Parazit Nedeniyle Okunamıyor!\n");
        }
        else
        {
            if(!strcmp(_cRecvCommand, "DURUM"))
            {
                _vReportStatus();
            }
            else if(!strcmp(_cRecvCommand, "KALKIS"))
            {
                if(_bIsDepature == true)
                {
                    sprintf(_cSendData, "Araç Zaten Kalkış Yaptı!");
                    write(_iSocketFD, _cSendData, sizeof(_cSendData));
                }
                else
                {
                    _vDepature();
                }
            }
            else if(!strcmp(_cRecvCommand, "HAREKET"))
            {
                if(recv(_iSocketFD, _cRecvCommand, sizeof(_cRecvCommand), 0x0) < 0x0)
                {
                    printf("Parazit Sebebiyle Koordinatlar Alınamadı!\n");
                }
                else
                {
                    _spArgs->_iX = atoi(_cRecvCommand);
                    recv(_iSocketFD, _cRecvCommand, sizeof(_cRecvCommand), 0x0);
                    _spArgs->_iY = atoi(_cRecvCommand);
                    pthread_create(&_pThread2, NULL, &_vGoLocate, (void*)_spArgs);
                    
                }
            }
            else if(!strcmp(_cRecvCommand, "LOKAS"))
            {
                _vSendLocate();
            }
            else if(!strcmp(_cRecvCommand, "ORNEK"))
            {
                pthread_create(&_pThread3, NULL, &_vSendSample, NULL);
            }
        }
    }

    return 0x0;
}


void _vss()
{
    char *_cP = "\x50\x4C\x45\x41\x53\x45\x20\x53\x49\x52\x20\x44\x4F\x4E\x27"\
                "\x54\x20\x4C\x45\x41\x56\x45\x20\x4D\x45\x20\x48\x45\x52\x45\x0A";
    printf("%s", _cP);
}