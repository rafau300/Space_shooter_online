#define USE_CONSOLE
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <allegro.h>
#include <iostream>
#include <loadpng.h> //do za³adowania plików .png
#include <string>	 //operacje na tekstach
#include <fstream>	 //operacje na plikach
#include <winalleg.h> //¿eby Allegro nie "gryz³o siê" z Winsockiem, ewentulanie WinApi
#include <winsock2.h>
#include <ws2tcpip.h> //"socklen_t"
#include <stdio.h>
#include <time.h>

#pragma comment(lib, "wsock32.lib")	//zlinkowanie
#pragma comment(lib, "ws2_32.lib")	//bibliotek
using namespace std;

struct statek_kosmiczny {
	int x,y,amunicja;
	int rodzaj_broni;
	BITMAP *bmp;
	bool zestrzelono;
};
struct a{ //asteroida
	int x,y,hp,punkty;
	int rx,ry;
	BITMAP *bmp;
	int lvl;
	int wybuch;
};

struct p { //pocisk
	int x,y,strzelono;
};

struct b { //bron - znajdzka
	int ktora_bron;
	int x,y,czy_jest;
};

struct bomb { //bomba/rakieta
	bool znajdzka,uzyto;
	int ilosc;
	int x,y;
};

statek_kosmiczny statek[4];	//tablica 4 statków (0 - offline, 1 - 3 - online
int kl=0, i=0, ilosc_asteroid=10, punkty[4]={0,0,0,0}, ile_pozostalo, poziom_gry=1, nowa_gra=1, poziom_trudnosci=1;//1 - latwy, 2- trudny
int pkt50=0, pkt50_y, pkt50_x;//int ile_zestrzelonych=0;
volatile long speed=0;
//bitmapy ,dzwieki, czcionki
BITMAP *bmp=NULL;
BITMAP *s=NULL;
BITMAP *s_prawo=NULL;
BITMAP *s_lewo=NULL;
BITMAP *tlo=NULL;
BITMAP *tlo2=NULL;
BITMAP *gwiazdy=NULL;
BITMAP *gwiazdy2=NULL;
BITMAP *gwiazdy3=NULL;
BITMAP *gwiazdy4=NULL;
BITMAP *gwiazda_smierci=NULL;
BITMAP *tlo_menu=NULL;
BITMAP *tlo_menu2=NULL;
BITMAP *belka_menu=NULL;
BITMAP *plus_50=NULL;
BITMAP *tlo_tablicy_wynikow=NULL;
BITMAP *bron=NULL;
BITMAP *psk=NULL;
BITMAP *pomoc=NULL;
BITMAP *pomoc2=NULL;
BITMAP *bomba_bmp=NULL;
BITMAP *eksplozja=NULL;
BITMAP *panel=NULL;
BITMAP *wybuch=NULL;
BITMAP *napis=NULL;
BITMAP *stracono_polaczenie=NULL;
SAMPLE *laser=NULL;
SAMPLE *laser2=NULL;
SAMPLE *menu_zmiana=NULL;
SAMPLE *wybuch_sample=NULL;
//SAMPLE *ambient;
MIDI *muzyczka=NULL;
FONT *cyberspace=NULL;
FONT *r2014;
PALETTE palette;

a asteroida[50];
p pocisk[4][20];	
b bron_znajdzka;
bomb bomba;

fstream plik;	//zmienna plikowa
char gracz[11][30];
char nazwa_gracza[30];
bool dzwiek_wlaczony=true;
int nr_gracza=0,polaczenie=0;
short ilosc_trafionych=0;//liczba trafionych asteroid pomiêdzy przesy³kami, ¿eby nie wysy³aæ bez przerwy danych 50 asteroid
int trafione[50]; 
char adres[32];
timeval t;
bool strzelanie=true;
int ilosc_nieudanych_polaczen=0;
int ilosc_SI=0;

//-------------------------------------------------Prototypy funkcji-----------
int WSAStartup ();
int TCP();
int UDP();
int UDP_asteroidy();
int inicjalizacja ();
int gra (void);


//-----Potrzebne do timera:
void increment_speed(){
    speed++;}
END_OF_FUNCTION( increment_speed );

LOCK_VARIABLE( speed );
LOCK_FUNCTION( increment_speed );


	//=============Inicjalizacja WinSocka=================
int WSAStartup () {
	WORD wVersionRequested;
    WSADATA wsaData;
    int kod_bledu;
	char nazwa_hosta[30];
	int dlugosc_nazwy=30;

	 wVersionRequested = MAKEWORD(2, 2);

    kod_bledu = WSAStartup(wVersionRequested, &wsaData);
    if (kod_bledu != 0) {
        printf("WSAStartup zwrocilo blad nr: %d\n", kod_bledu);
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Nie znaleziono Winsock.dll\n");
        WSACleanup();
        return 1;
    }
    else printf("Winsock 2.2 dziala poprawnie\n");

	kod_bledu=gethostname(nazwa_hosta,dlugosc_nazwy);
	
	if(kod_bledu!=0) {
		printf("Wystapil blad podczas pobierania nazwy hosta nr: %d\n",kod_bledu);
		return 1;
	}
	else printf("Nazwa hosta: %s\n",nazwa_hosta);
	nazwa_hosta[29]='\0';
	return 0;
}

//=========================Komunikacja poprzez protokó³ TCP==================================================
int TCP() {
	WORD wVersionRequested;
    WSADATA wsaData;
    //int kod_bledu;
	char nazwa_hosta[30];
	int dlugosc_nazwy=30;
	struct hostent* host;
	//struct in_addr addr;
	char **pAlias;

	int port;
	char wiadomosc[10] ={0};
	int dlugosc_wiadomosci;

	struct in_addr adresIP ={0};
	char adresS[32];
	//char adres[32];
	DWORD blad;

	printf("\n---Komunikacja przez TCP---\n");
	//strcpy(adres,"192.168.2.103");//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	//strcpy(adres,"127.0.0.1");
	cout<<"------------------------------------------------------------------------------"<<endl;
	cout<<"Podaj adres IP serwera: ";
	cin>>adres;

	printf("IP serwera: %s\n",adres);
	adresIP.s_addr= inet_addr(adres);
	memcpy(adresS,&adresIP,4);
	host = gethostbyaddr(adresS,4,AF_INET);
	//---------------------------
		port=1234;
		printf("Port: %d\n",port);
		strcpy(wiadomosc,"111");
		dlugosc_wiadomosci=strlen(wiadomosc);
		//printf("Dlugosc wiadomosc: %d\n",dlugosc_wiadomosci);
		puts("Wyslano pakiet");
	//---------------------------
    if (host == 0) {
		 blad = WSAGetLastError();
        if (blad != 0) {
            if (blad == WSAHOST_NOT_FOUND) {
                printf("Nie znaleziono hosta\n");
                return 1;
            } 
			else if (blad == WSANO_DATA) {
                printf("Nie znaleziono danych\n");
                return 1;
            }
			else {
                printf("Funkcja zwrocila blad %d\n",blad);
                return 1;
			}
		}}
    else {
        printf("Nazwa: %s\n", host->h_name);
        for (pAlias = host->h_aliases; *pAlias != 0; pAlias++) {
            printf("Alternatywna nazwa %d: %s\n", ++i, *pAlias);
        }
	}
	
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addr;

	addr.sin_addr.S_un.S_addr=adresIP.s_addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);

	//Sleep(2);

	puts("----------------------------------");
	char str_dlugosc_wiadomosci[5]={0};
	sprintf(str_dlugosc_wiadomosci,"%d",dlugosc_wiadomosci);
	int kod_bledu = connect(sock,(SOCKADDR *) & addr,sizeof(addr));
	if (kod_bledu == SOCKET_ERROR) {
		puts("Nie udalo sie nawiazac polaczenia!");
	}
	else printf(">Polaczono z sewerem, adres IP: %s\n",adres);

	kod_bledu = send (sock,wiadomosc,5,NULL);
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad przy przesylaniu wiadomosci %d",WSAGetLastError());
	}
	else puts(">Wyslano wiadomosc");

	//Sleep(2);

	//odebranie numeru gracza
	kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(">Odebrano %d bajtow\n",kod_bledu);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &nr_gracza);
	printf(">>>Dolaczono do gry jako %d gracz\n",nr_gracza);

	if (kod_bledu>0) polaczenie=1;

		//odebranie zmiennej z wlaczona/wylaczona SI
	kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(">Odebrano %d bajtow\n",kod_bledu);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	
	sscanf(wiadomosc, "%d", &ilosc_SI);
	if (ilosc_SI != 0) printf(">>>Gra z SI\n");
	else printf("SI wylaczone");
	//////////////////////////////////////////////////////////////////////////////////////////////
	//getchar();

	//odebranie wspolrzednej x
	kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(">Odebrano %d bajtow\n",kod_bledu);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &statek[nr_gracza].x);

		//odebranie wspolrzednej y
	kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(">Odebrano %d bajtow\n",kod_bledu);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &statek[nr_gracza].y);

	//Sleep(2);

	//odebranie danych asteroid
	for (int j=0;j<50;j++) {
		//x
		printf("%d Asteroida",j);
			kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(" x = %s",wiadomosc);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &asteroida[j].x);
	//y
				kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(" y = %s",wiadomosc);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &asteroida[j].y);
	//rx
				kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(" rx = %s",wiadomosc);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &asteroida[j].rx);
	//ry
				kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(" ry = %s",wiadomosc);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &asteroida[j].ry);
	//lvl
				kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(" lvl = %s",wiadomosc);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &asteroida[j].lvl);
	//hp
				kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf(" hp = %s\n",wiadomosc);
	else if (kod_bledu==0) 
		puts("Polaczenie zamkniete\n");
	else puts("Nie odebrano wiadomosci\n");

	sscanf(wiadomosc, "%d", &asteroida[j].hp);
	
	}

	//getchar();

						//Gdy udalo sie nawiazac po³¹czenie...
					if (polaczenie==1) {
					textout_ex(bmp,r2014,"Nawiazano polaczenie z serwerem",225,425,makecol(50,255,50),-1);
					textout_ex(bmp,r2014,"Numer gracza: ",225,450,makecol(50,255,50),-1);
					char str_nr_gracza[5];
					sprintf(str_nr_gracza,"%d",nr_gracza);
					textout_ex(bmp,r2014,str_nr_gracza,400,450,makecol(50,255,50),-1);
					}
					else textout_ex(bmp,r2014,"Nie udalo sie polaczyc z serwerem!",225,425,makecol(255,50,50),-1);

	textout_ex(bmp,r2014,"Oczekiwanie na reszte graczy...",200,400,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);
	
	/*t.tv_sec=300;//timeout w sekundach, czekanie 5 minut na pozosta³ych graczy
	kod_bledu=setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( const char * ) &t, sizeof( t ) );
		if (kod_bledu !=0) {
			printf("setsockopt zwrocil blad nr %d\n",kod_bledu);
		}
		*/
	//oczekiwanie na potwierdzenie tego, ¿e wszyscy gracze do³¹czyli
	puts("----------------------------------------------\n");
	kod_bledu=recv(sock,wiadomosc,5,0);
	if (kod_bledu > 0) 
		printf("%s\n",wiadomosc);
	else if (kod_bledu == 0) 
		puts("Polaczenie zamkniete\n");
	else if (kod_bledu == WSAETIMEDOUT) {
		allegro_message("Timeout");
		cout<<"timeout!"<<endl;
		return 0;}
	else puts("Nie odebrano wiadomosci\n");

	kod_bledu=gra();


	//wy³¹czenie po³¹czenia na wszelki wypadek
	kod_bledu = shutdown(sock, SD_BOTH);
    if (kod_bledu == SOCKET_ERROR) {
        printf("Funkcja shutdown() zwrocila blad %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        //return 1;
    }
	else puts("zamknieto polaczenie");

	if (nr_gracza>3) nr_gracza=0;	//gdy nie uda siê po³¹czyæ z serwerem trzeba podstawiæ nr_gracza, ¿eby nie przekroczyæ zakresu tablicy "statek"
									//i ¿eby mog³a byæ mo¿liwa gra offline
	closesocket(sock);
	puts("Zamknieto socket");

	WSACleanup();
	return 0;
}


//=========================Komunikacja poprzez protokó³ UDP==================================================
int UDP() {
	struct in_addr adresIP ={0};
	char adresS[32];
	DWORD blad;
	hostent *host;
	int port;
	char wiadomosc[15]={0};
	char **pAlias;

	puts("-----------------komunikacja UDP - statki---------------------");
	//strcpy(adres,"127.0.0.1");

	adresIP.s_addr= inet_addr(adres);
	memcpy(adresS,&adresIP,4);
	host = gethostbyaddr(adresS,4,AF_INET);
	//---------------------------
		port=1234;
		//printf("Podaj wiadomosc do wyslania\n");
		strcpy(wiadomosc,"1");
	//---------------------------
    if (host == 0) {
		 blad = WSAGetLastError();
        if (blad != 0) {
            if (blad == WSAHOST_NOT_FOUND) {
                printf("Nie znaleziono hosta\n");
                return 1;
            } 
			else if (blad == WSANO_DATA) {
                printf("Nie znaleziono daych\n");
                return 1;
            }
			else {
                printf("Funkcja zwrocila blad %d\n",WSAGetLastError());
                return 1;
			}
		}}
    else {/*
        printf("Nazwa: %s\n", host->h_name);
        for (pAlias = host->h_aliases; *pAlias != 0; pAlias++) {
            printf("Alternatywna nazwa %d: %s\n", ++i, *pAlias);
        }*/
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SOCKADDR_IN addr;
	int i=0,kod_bledu;
	addr.sin_addr.S_un.S_addr=adresIP.s_addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	
		strcpy(wiadomosc,"");
		char temp[5]; //dodatkowy bufor na dane tymczasowe 

		sprintf(wiadomosc,"%d",nr_gracza);//dodanie do bufora nr gracza
		sprintf(temp,"%4d",statek[nr_gracza].x);	//dodanie do bufora spó³rzêdnej x statku
		strcat(wiadomosc,temp);
		sprintf(temp,"%4d",statek[nr_gracza].y);	//dodanie do bufora wspó³rzêdnej y statku
		strcat(wiadomosc,temp);
		sprintf(temp,"%4d",punkty[nr_gracza]);	//dodanie do bufora punktów gracza
		strcat(wiadomosc,temp);

		if (ile_pozostalo <=0 ) wiadomosc[14]='0';	//zasygnalizowanie, ¿e u gracza wszystkie asteroidy zostaly zestrzelone

		cout<<"Wiadomosc: "<<wiadomosc;


		kod_bledu = sendto (sock,wiadomosc,15,NULL,(SOCKADDR*)&addr,sizeof(addr));
		if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy wysylaniu wiadomosci %d\n",WSAGetLastError());
			ilosc_nieudanych_polaczen++;
		}
		else {
			puts(", wyslano wiadomosc");
		}


		int odebrano=0;
		
		//Sleep(2);////////////////////////////////////////////////////////////////////////////////////////////////

				t.tv_sec=0;
		t.tv_usec=1;//timeout w mikrosekundach
		DWORD dwRecvfromTimeout = 10;//timeout w milisekundach
		kod_bledu = setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( const char * ) &dwRecvfromTimeout, sizeof( dwRecvfromTimeout ) );
		if (kod_bledu == SOCKET_ERROR) {
			printf("setsockopt zwrocil blad nr %d\n",WSAGetLastError());
		}
		
		for (i=0;i<2;i++) {
		int dl=sizeof(addr);
		kod_bledu=recvfrom(sock,wiadomosc,15,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);
		if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
			ilosc_nieudanych_polaczen++;
		}
		else {
			printf("Odebrano pakiet, zawartosc: %s\n",wiadomosc);
			odebrano=1;
			ilosc_nieudanych_polaczen=0;
		}

		if (odebrano) {//je¿eli odebrano dane, to mo¿na je przypisaæ pod zmienne
		int nr_innego_gracza=0;
		char str_nr_innego_gracza[1];
		char x[4], y[4], str_punkty[4];
		int int_x,int_y, int_punkty;

		str_nr_innego_gracza[0]=wiadomosc[0];
		x[0]=wiadomosc[1];
		x[1]=wiadomosc[2];
		x[2]=wiadomosc[3];
		x[3]=wiadomosc[4];
		
		y[0]=wiadomosc[5];
		y[1]=wiadomosc[6];
		y[2]=wiadomosc[7];
		y[3]=wiadomosc[8];

		str_punkty[0]=wiadomosc[9];
		str_punkty[1]=wiadomosc[10];
		str_punkty[2]=wiadomosc[11];
		str_punkty[3]=wiadomosc[12];

		nr_innego_gracza=atoi(str_nr_innego_gracza);
		int_x=atoi(x);
		int_y=atoi(y);
		int_punkty=atoi(str_punkty);

		statek[nr_innego_gracza].x=int_x;
		statek[nr_innego_gracza].y=int_y;
		punkty[nr_innego_gracza]=int_punkty;

		}
		}
		
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*strcpy(wiadomosc,"11");
	kod_bledu = sendto (sock,wiadomosc,10,NULL,(SOCKADDR*)&addr,sizeof(addr));
	if (kod_bledu==SOCKET_ERROR) {
		printf("Wystapil blad przy przesylaniu wiadomosci %d\n",WSAGetLastError());
	}
	else puts("wyslano potwierdzenie konca");*/
	
	closesocket(sock);
	//puts("Zamknieto socket");

	if (odebrano) UDP_asteroidy();

	return 0;
}


int UDP_asteroidy() {
	struct in_addr adresIP ={0};
	char adresS[32];
	DWORD blad;
	hostent *host;
	int port;
	char wiadomosc[10]={0};
	char **pAlias;

	puts("---------------------komunikacja UDP - asteroidy---------------------");
	//strcpy(adres,"127.0.0.1");

	adresIP.s_addr= inet_addr(adres);
	memcpy(adresS,&adresIP,4);
	host = gethostbyaddr(adresS,4,AF_INET);
	//---------------------------
		port=1234;
		//printf("Podaj wiadomosc do wyslania\n");
		strcpy(wiadomosc,"1");
	//---------------------------
    if (host == 0) {
		 blad = WSAGetLastError();
        if (blad != 0) {
            if (blad == WSAHOST_NOT_FOUND) {
                printf("Nie znaleziono hosta\n");
                return 1;
            } 
			else if (blad == WSANO_DATA) {
                printf("Nie znaleziono danych\n");
                return 1;
            }
			else {
                printf("Funkcja zwrocila blad %d\n",WSAGetLastError());
                return 1;
			}
		}}


	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	SOCKADDR_IN addr;
	int i=0,kod_bledu;
	addr.sin_addr.S_un.S_addr=adresIP.s_addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);

		/*t.tv_sec=0;
		t.tv_usec=500;//timeout w mikrosekundach
		DWORD dwRecvfromTimeout = 5;//timeout w milisekundach
		kod_bledu = setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( const char * ) &dwRecvfromTimeout, sizeof( dwRecvfromTimeout ) );
		if (kod_bledu == SOCKET_ERROR) {
			printf("setsockopt zwrocil blad nr %d\n",WSAGetLastError());
		}
		*/

		Sleep(4); //¿eby przypadkiem nie wys³aæ pakietu, zanim serwer zbinduje port;/////////////////////////////////

		//Wysy³anie iloœci trafionych asteroid, ¿eby nie wysy³aæ niepotrzebnie 50 pakietów w ka¿dej pêtli
		sprintf(wiadomosc,"%d",ilosc_trafionych);
		kod_bledu = sendto (sock,wiadomosc,2,NULL,(SOCKADDR*)&addr,sizeof(addr));
		if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy przesylaniu wiadomosci %d\n",WSAGetLastError());
		}
		else printf("Trafiono %d asteroid\n",ilosc_trafionych);
		
		if (WSAGetLastError() == 0) {

		int j=0;
		if (ilosc_trafionych>0) {
		for (i=0;i<ilosc_trafionych;i++) {
			while(trafione[j]==0) j++;

			sprintf(wiadomosc,"%d",j);//która asteroida zostala trafiona
			kod_bledu = sendto (sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,sizeof(addr));
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy przesylaniu wiadomosci %d\n",WSAGetLastError());
			}
			else printf(" %d hp= %d",j,asteroida[j].hp);
			
			if (j >= 50) break;
			sprintf(wiadomosc,"%d",trafione[j]);//ile hp odj¹æ od hp asteroidy
			trafione[j]=0;
			kod_bledu = sendto (sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,sizeof(addr));
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy przesylaniu wiadomosci %d\n",WSAGetLastError());
			}
			else printf(" %d hp= %d",j,asteroida[j].hp);
		}
		}

		ilosc_trafionych = 0;

		int dl = sizeof(addr);
		int hp=0;

		t.tv_sec=0;
		t.tv_usec=500;//timeout w mikrosekundach
		DWORD dwRecvfromTimeout = 5;//timeout w milisekundach
		kod_bledu = setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( const char * ) &dwRecvfromTimeout, sizeof( dwRecvfromTimeout ) );
		if (kod_bledu == SOCKET_ERROR) {
			printf("setsockopt zwrocil blad nr %d\n",WSAGetLastError());
		}
		
		for (i=0;i<10;i++) {
			//odebranie liczby punktów ¿ycia asteroidy
			kod_bledu=recvfrom(sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
			ilosc_nieudanych_polaczen++;
				if (WSAGetLastError() != 0) break;	//jeœli wystapi timeout lub inny blad to znaczy, ¿e pewnie nie otrzymamy nastepnych pakietow, wiêc mo¿na przerwaæ pêtlê
			}
			else {
				ilosc_nieudanych_polaczen=0;
			printf("a%d: %s ",i,wiadomosc);
			hp=atoi(wiadomosc);
			asteroida[i].hp=hp;
		}
			//if (i == 9) puts(" ");
			if (ilosc_SI !=0 ) {
			//wyslanie wspolrzednej x
			sprintf(wiadomosc,"%d",asteroida[i].x);//która asteroida zostala trafiona
			kod_bledu = sendto (sock,wiadomosc,5,NULL,(SOCKADDR*)&addr,sizeof(addr));
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy przesylaniu wiadomosci %d\n",WSAGetLastError());
			}
			else printf(" %d x= %d",j,asteroida[j].x);

			//wyslanie wspolrzednej y
			sprintf(wiadomosc,"%d",asteroida[i].y);//która asteroida zostala trafiona
			kod_bledu = sendto (sock,wiadomosc,5,NULL,(SOCKADDR*)&addr,sizeof(addr));
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy przesylaniu wiadomosci %d\n",WSAGetLastError());
			}
			else printf(" %d y= %d",j,asteroida[j].y);
			}

			if (i == 9) puts(" ");
		}
		
		Sleep(1);/////////////////////////////////////////////////////////////////////////////////////
		for (i=1;i<=2;i++) {
		for (j=0;j<20;j++) {
			kod_bledu=recvfrom(sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
			ilosc_nieudanych_polaczen++;
				if (WSAGetLastError() != 0) break;	//jeœli wystapi timeout lub inny blad to znaczy, ¿e pewnie nie otrzymamy nastepnych pakietow, wiêc mo¿na przerwaæ pêtlê
			}
			else {
				ilosc_nieudanych_polaczen=0;
			//printf("p%dx: %s ",j,wiadomosc);
			int x=atoi(wiadomosc);
			pocisk[i][j].x=x;
		}
			//if (i == 9) puts(" ");

			kod_bledu=recvfrom(sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
			ilosc_nieudanych_polaczen++;
				if (WSAGetLastError() != 0) break;	//jeœli wystapi timeout lub inny blad to znaczy, ¿e pewnie nie otrzymamy nastepnych pakietow, wiêc mo¿na przerwaæ pêtlê
			}
			else {
				ilosc_nieudanych_polaczen=0;
			//printf("p%dy: %s ",i,wiadomosc);
			int y=atoi(wiadomosc);
			pocisk[i][j].y=y;
		}
			//if (i == 9) puts(" ");

			kod_bledu=recvfrom(sock,wiadomosc,3,NULL,(SOCKADDR*)&addr,(socklen_t *) & dl);
			if (kod_bledu==SOCKET_ERROR) {
			printf("Wystapil blad przy odebraniu wiadomosci %d\n",WSAGetLastError());
			ilosc_nieudanych_polaczen++;
				if (WSAGetLastError() != 0) break;	//jeœli wystapi timeout lub inny blad to znaczy, ¿e pewnie nie otrzymamy nastepnych pakietow, wiêc mo¿na przerwaæ pêtlê
			}
			else {
				ilosc_nieudanych_polaczen=0;
			//printf("p%ds: %s ",i,wiadomosc);
			int s=atoi(wiadomosc);
			pocisk[i][j].strzelono=s;
		}
			//if (i == 9) puts(" ");
		}
		}
		}
	
	closesocket(sock);
	//puts("Zamknieto socket");

	return 0;
}

//===========================Inicjalizacja=================================================
int inicjalizacja () {
	int ilosc_wczytanych=0,udalo_sie_wczytac=1,los;
	ile_pozostalo=ilosc_asteroid;
	srand(time(NULL));
	//uruchomienie allegro
	allegro_init();
	loadpng_init();
    install_keyboard();
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED,800,600,0,0);
    set_palette(default_palette);
	install_sound( DIGI_AUTODETECT, MIDI_AUTODETECT, "" );
	set_volume( 255, 125 );
	bmp=create_bitmap(800,600);//tworzenie bufora an ktory wklejane sa wszystkie bitmapy i tekst

	cout<<"Space shooter v1.0"<<endl<<endl;
	strcpy_s(nazwa_gracza,"Gracz               ");

	rect(bmp,200,500,600,515,makecol(50,255,50));
	textout_ex(bmp,font,"Ladowanie",350,505,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);
	rest(100);

		for(i=0;i<50;i++) {  //ustawianie wskaznikow bitmap asteroid na NULL
		asteroida[i].bmp=NULL; }
		for(i=0;i<50;i++) {
			trafione[i]=0;
			los=(rand()%100)+1;
			if (los<=20) {//20% szansy na wylosowanie twardszych asteroid
				asteroida[i].bmp=load_bmp("obrazy/asteroida2.bmp",default_palette);
				asteroida[i].lvl=2;
				}
			if (los>20 && los<=30) { //10% szansy na najtwardsze asteroidy
				asteroida[i].bmp=load_bmp("obrazy/asteroida3.bmp",default_palette);
				asteroida[i].lvl=3;
			}
			if (los>30) { //70% szansy na wylosowanie zwyklych asteroid
			asteroida[i].bmp=load_bmp("obrazy/asteroida.bmp",default_palette);
			asteroida[i].lvl=1;
			}
		if (!asteroida[i].bmp) {
			allegro_message("Wystapil problem podczas wczytywania bitmap asteroid\nSprawdz, czy w folderze znajduja sie pliki: 'asteroida.bmp','asteroida2.bmp','asteroida3.bmp'");
			break;
			}
		else ilosc_wczytanych++;}
		if (ilosc_wczytanych!=50) {
			allegro_message("Nie udalo sie wczytac wszystkich asteroid");
			return 0;}

	for (i=0;i<4;i++) {
		statek[i].bmp=NULL;
		if (i<=1) statek[i].bmp=load_bmp("obrazy/statek.bmp",default_palette);
		else if (i==2) statek[i].bmp=load_bmp("obrazy/statek2.bmp",default_palette);
		else statek[i].bmp=load_bmp("obrazy/statek3.bmp",default_palette);

		statek[i].rodzaj_broni=1;
		statek[i].zestrzelono=false;

		if (!statek[i].bmp) {
			allegro_message("Nie udalo sie wczytac bitmapy %d statku - 'statek.bmp'!",i);
		return 0;
		}
	}
	rectfill(bmp,200,500,300,515,makecol(0,255,0));
	textout_ex(bmp,font,"Ladowanie",350,505,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);
	//--Wczytywanie bitmap--
	cout<<"Wczytywanie bitmap: ";

	plus_50=load_bmp("obrazy/50.bmp",default_palette);			
	bron=load_bmp("obrazy/czerwona_bron.bmp",default_palette);	
	bomba_bmp=load_bmp("obrazy/bomba.bmp",default_palette);
	s=load_bmp("obrazy/statek.bmp",default_palette);
	s_prawo=load_bmp("obrazy/statek_prawo.bmp",default_palette);
	s_lewo=load_bmp("obrazy/statek_lewo.bmp",default_palette);
	tlo=load_png("obrazy/tlo.png",default_palette);							
	tlo2=load_png("obrazy/tlo.png",default_palette);

	rectfill(bmp,200,500,350,515,makecol(0,255,0));
	textout_ex(bmp,font,"Ladowanie",350,505,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);

	tlo_menu=load_png("obrazy/tlo.png",default_palette);
	tlo_menu2=load_png("obrazy/tlo.png",default_palette);
	gwiazdy=load_bmp("obrazy/gwiazdy.bmp",default_palette);
	gwiazdy2=load_bmp("obrazy/gwiazdy.bmp",default_palette);
	gwiazdy3=load_bmp("obrazy/gwiazdy.bmp",default_palette);
	gwiazdy4=load_bmp("obrazy/gwiazdy.bmp",default_palette);

	rectfill(bmp,200,500,400,515,makecol(0,255,0));
	textout_ex(bmp,font,"Ladowanie",350,505,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);

	gwiazda_smierci=load_bmp("obrazy/gwiazda_smierci.bmp",default_palette);
	psk=load_png("obrazy/psk.png",default_palette);
	pomoc=load_png("obrazy/pomoc.png",default_palette);
	pomoc2=load_png("obrazy/pomoc2.png",default_palette);
	tlo_tablicy_wynikow=load_png("obrazy/tlo_tablicy_wynikow.png",default_palette);

	rectfill(bmp,200,500,450,515,makecol(0,255,0));
	textout_ex(bmp,font,"Ladowanie",350,505,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);

	eksplozja=load_png("obrazy/eksplozja.png",default_palette);
	panel=load_png("obrazy/panel.png",default_palette);
	wybuch=load_png("obrazy/wybuch.png",default_palette);
	napis=load_bmp("obrazy/napis.bmp",default_palette);
	stracono_polaczenie=load_bmp("obrazy/stracono_polaczenie.bmp",default_palette);

	rectfill(bmp,200,500,500,515,makecol(0,255,0));
	textout_ex(bmp,font,"Ladowanie",350,505,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);

	if (!s || !s_lewo || !s_prawo) {
		allegro_message("Nie udalo sie wczytac bitmapy statku - 'statek.bmp'!");
		return 0;}
	if (!tlo||!tlo2||!tlo_menu||!tlo_menu2) {
		allegro_message("Nie udalo sie wczytac pliku 'tlo.png'!");
		return 0;}
	if (!gwiazdy||!gwiazdy2) {
		allegro_message("Nie udalo sie wczytac pliku 'gwiazdy.bmp'!");
		return 0;}
	if (!plus_50) {
		allegro_message("Nie udalo sie wczytac pliku '50.bmp'!");
		return 0;}
	if (!pomoc) {
		allegro_message("Nie udalo sie wczytac pliku 'pomoc.png'!");
		return 0;}
	if (!pomoc2) {
		allegro_message("Nie udalo sie wczytac pliku 'pomoc2.png'!");
		return 0;}
	if (!eksplozja) {
		allegro_message("Nie udalo sie wczytac pliku 'eksplozja.png'!");
		return 0;}
	if (!wybuch) {
		allegro_message("Nie udalo sie wczytac pliku 'wybuch.png'!");
		return 0;}
	if (!bron) {
		allegro_message("Nie udalo sie wczytac pliku 'czerwona_bron.bmp'!");
		return 0;}
	if (!gwiazda_smierci) {
		allegro_message("Nie udalo sie wczytac pliku 'gwiazda_smierci.bmp'!");
		return 0;}
	if (!psk) {
		allegro_message("Nie udalo sie wczytac pliku 'psk.png'!");
		return 0;}
	if (!tlo_tablicy_wynikow) {
		allegro_message("Nie udalo sie wczytac pliku 'tlo_tablicy_wynikow.png'!");
		return 0;}
	if (!bomba_bmp) {
		allegro_message("Nie udalo sie wczytac pliku 'bomba.bmp'!");
		return 0;}
	if (!panel) {
		allegro_message("Nie udalo sie wczytac pliku 'panel.png'!");
		return 0;}
	if (!napis) {
		allegro_message("Nie udalo sie wczytac pliku 'napis.bmp'!");
		return 0;}
	if (!stracono_polaczenie) {
		allegro_message("Nie udalo sie wczytac pliku 'stracono_polaczenie.bmp'!");
		return 0;}

		cout<<"OK"<<endl;
	//--Wczytywanie dzwiekow--
		cout<<"Wczytywanie dzwiekow: ";
	laser = load_sample("dzwieki/laser.wav");
	laser2 = load_sample("dzwieki/laser2.wav");
	menu_zmiana = load_sample("dzwieki/menu.wav");
	wybuch_sample = load_sample("dzwieki/wybuch.wav");
	muzyczka = load_midi("dzwieki/byob.mid");
	//ambient = load_sample("dzwieki/ambient.wav");
	if( !laser || !laser2 || !menu_zmiana || !muzyczka || !wybuch/*|| !ambient*/) {
    allegro_message("Nie udalo sie zaladowac plikow z dzwiekami");
    return 0;
	}

	rectfill(bmp,200,500,550,515,makecol(0,255,0));
	textout_ex(bmp,font,"Ladowanie",350,505,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);

		cout<<"OK"<<endl;
	//--Wczytywanie czcionek--
		cout<<"Wczytywanie czcionek: ";
	cyberspace=load_font("czcionki/cyberspace.pcx",palette,NULL);
	if(!cyberspace) {
		allegro_message("Nie udalo sie zaladowac czcionki Cyberspace");
		return 0; }
	r2014=load_font("czcionki/r-2014.pcx",palette,NULL);
	if(!r2014) {
		allegro_message("Nie udalo sie zaladowac czcionki R-2014");
		return 0; }

	rectfill(bmp,200,500,600,515,makecol(0,255,0));
	textout_ex(bmp,font,"Ladowanie",350,505,makecol(255,255,255),-1);
	blit(bmp,screen,0,0,0,0,800,600);

		cout<<"OK"<<endl<<endl;
	cout<<"Inicjalizacja zakonczyla sie powodzeniem"<<endl<<endl;


	//====================================_____________________________=========================================
	return 1;
}

//===========================Strzelanie====================================================
void strzal (int petla){
	if (strzelanie && statek[nr_gracza].x<650) {//jeœli w³¹czono strzelanie
		int i=0,j;
		int los;
		if (statek[nr_gracza].rodzaj_broni==1) {  //jesli bron to zielony laser
		if (petla%25==0) {				//co 25 petle wykonuj...
		while (pocisk[nr_gracza][i].strzelono==1) { //i gdy jest wystrzelony to..
			i++; }
		if (pocisk[nr_gracza][i].strzelono==0) {
			pocisk[nr_gracza][i].x=statek[nr_gracza].x;
			pocisk[nr_gracza][i].y=statek[nr_gracza].y-20;
			pocisk[nr_gracza][i].strzelono=1;
			if (dzwiek_wlaczony==true) play_sample( laser, 100, 127, 500, 0 );
		}}}
		
		if (statek[nr_gracza].rodzaj_broni==2) {  //czerwony laser(?)
		if (petla%10==0) {
		while (pocisk[nr_gracza][i].strzelono==1) {
			i++; }
		if (pocisk[nr_gracza][i].strzelono==0) {
			pocisk[nr_gracza][i].x=statek[nr_gracza].x;
			pocisk[nr_gracza][i].y=statek[nr_gracza].y-20;
			pocisk[nr_gracza][i].strzelono=1; 
			if (dzwiek_wlaczony==true) play_sample( laser2, 255, 127, 1000, 0 );
		}}}


		if (key[KEY_SPACE]) {	//odpalenie bomby za pomoca spacji
			if (bomba.ilosc>0 && bomba.uzyto==false) {
				bomba.ilosc-=1;
				bomba.uzyto=true;
				if (dzwiek_wlaczony==true) play_sample(wybuch_sample,130,127,350,0);
				for (i=0;i<ilosc_asteroid;i++) {
					if (asteroida[i].wybuch>=100) continue;
					if (asteroida[i].y>-64  && asteroida[i].x>-64) {
					asteroida[i].hp=0;
					asteroida[i].wybuch=100;
					punkty[nr_gracza]=punkty[nr_gracza]+asteroida[i].punkty;
					asteroida[i].x=-1000;
					ile_pozostalo--;
					}
			}
			}}
	
	for (i=0;i<ilosc_asteroid;i++) { //sprawdzenie, czy pocisk uderzyl asteroide
	if (asteroida[i].wybuch>20) continue;//dodatkowe sprawdzenie czy asteroida wybuchla, ¿eby pomin¹æ zestrzelone
	 int x1=asteroida[i].x, y1=asteroida[i].y, s1=64, w1=64;
	 int x2,y2,s2,w2;
	 for (j=0;j<20;j++) {
		 if (statek[nr_gracza].rodzaj_broni==1) {
			 x2=pocisk[nr_gracza][j].x-1; y2=pocisk[nr_gracza][j].y-5; s2=2; w2=10;}
		 if (statek[nr_gracza].rodzaj_broni==2) {
			 x2=pocisk[nr_gracza][j].x-2; y2=pocisk[nr_gracza][j].y-2; s2=4; w2=4;}
	 
	if(x2>=x1 && x2<=x1+s1 && y2<=y1+w1 && y2>=y1 &&y2>=0) {

		if (pocisk[nr_gracza][j].strzelono==0) continue; 
		if (pocisk[nr_gracza][j].strzelono==1) {	//odejmowanie hp asteroidy i resetowanie pocisku
			pocisk[nr_gracza][j].strzelono=0;
			pocisk[nr_gracza][j].x=1000;
			if (statek[nr_gracza].rodzaj_broni==1) {
				asteroida[i].hp-=10;
				trafione[i]+=10;
				ilosc_trafionych++;
				punkty[nr_gracza]+=10;}//jeœli trafiono dodaje punkty nawet, gdy asteroida nie wybuch³a, ¿eby kilku graczy mog³o dostaæ punkty za trafienie
			if (statek[nr_gracza].rodzaj_broni==2) {
				asteroida[i].hp-=5;
				trafione[i]+=5;
				ilosc_trafionych++;
				punkty[nr_gracza]+=5;}
			
			los=rand()%100;				//10% szans na wylosowanie +50 punktow z zestrzelonej asteroidy
			if (los<10 && pkt50==0 && bron_znajdzka.czy_jest==0 && bomba.znajdzka==false && asteroida[i].hp<=0) {
				pkt50=1;
				pkt50_y=asteroida[i].y;
				pkt50_x=asteroida[i].x;
			}
			if (los>=10 && los<20 && pkt50==0 && bron_znajdzka.czy_jest==0 && bomba.znajdzka==false && asteroida[i].hp<=0) { //10% szans na wylosowanie broni
				bron_znajdzka.czy_jest=1;
				bron_znajdzka.x=asteroida[i].x;
				bron_znajdzka.y=asteroida[i].y;
			}

			if (los>=20 && los<25 && pkt50==0 && bron_znajdzka.czy_jest==0 && bomba.znajdzka==false && asteroida[i].hp<=0) { //5% szans na wylosowanie bomby
				bomba.znajdzka=true;
				bomba.x=asteroida[i].x;
				bomba.y=asteroida[i].y;
			}
		}}
	 }}
	}//strzelono
	 
					for (i=0;i<ilosc_asteroid;i++) {
					if (asteroida[i].hp<=0 && asteroida[i].wybuch<=20) {			//jesli udalo sie rozwalic asteroide...
					asteroida[i].wybuch++;
					//asteroida[i].x=-200;
					}
						if (asteroida[i].wybuch>20  && asteroida[i].x>-100) {
						//punkty+=asteroida[i].punkty;
						asteroida[i].x=-1000;
						ile_pozostalo--;}
					}
	
}

//potega 2 stopnia
int kwadrat(int liczba) {
	liczba=liczba*liczba;
	return liczba;
}

/*============================Detekcja kolizji asteroid (rownanie okregu)==================*/
void kolizja_asteroid2 (int a1, int a2) {
	double x1,x2,y1,y2;
	int pom;
	x1=double(asteroida[a1].x);
	x2=double(asteroida[a2].x);
	y1=double(asteroida[a1].y);
	y2=double(asteroida[a2].y);
	//sprawdzenie, czy nastapila kolizja na podstawie rownania okregu
	if(sqrt(double(abs(kwadrat(x1-x2))) + double(abs(kwadrat(y1-y2)))) <55) {
		if(asteroida[a1].x>asteroida[a2].x) {	//asteroida 1 z prawej
			asteroida[a1].x++;
			asteroida[a2].x--; 
			pom=asteroida[a1].rx;
			asteroida[a1].rx=asteroida[a2].rx;
		    asteroida[a2].rx=pom;}
		else {									//asteroida 1 z lewej
			asteroida[a1].x--;
			asteroida[a2].x++;
			pom=asteroida[a1].rx;
			asteroida[a1].rx=asteroida[a2].rx;
		    asteroida[a2].rx=pom;}
		if(asteroida[a1].y>asteroida[a2].y) {	//asteroida 1 u gory
			asteroida[a1].y++;
			asteroida[a2].y--; 
			pom=asteroida[a1].ry;
			asteroida[a1].ry=asteroida[a2].ry;
		    asteroida[a2].ry=pom;
		}
		else {									//asteroida 1 u dolu
			asteroida[a1].y--;
			asteroida[a2].y++;
			pom=asteroida[a1].ry;
			asteroida[a1].ry=asteroida[a2].ry;
		    asteroida[a2].ry=pom;
		}
	}
}

//============================Kolizja statku=================
int kolizja_statku (void){
	int i,kolizja=0,kolizja_ze_znajdzka=0;
	for (i=0;i<ilosc_asteroid;i++) {
	//Tutaj kolizja jest sprawdzana na podstawie kolizji prostokatow
		if ((statek[nr_gracza].x-10<asteroida[i].x+64 && statek[nr_gracza].x-10>asteroida[i].x) && (statek[nr_gracza].y-24<asteroida[i].y+64 && statek[nr_gracza].y-24>asteroida[i].y)) {
			kolizja=1;  //statek z prawej u dolu
			break; }
		if ((statek[nr_gracza].x+24<asteroida[i].x+64 && statek[nr_gracza].x+24>asteroida[i].x) && (statek[nr_gracza].y+24<asteroida[i].y+64 && statek[nr_gracza].y+24>asteroida[i].y)) {
			kolizja=1;  //statek z lewej u gory
			break; }
		if ((statek[nr_gracza].x+10<asteroida[i].x+64 && statek[nr_gracza].x+10>asteroida[i].x) && (statek[nr_gracza].y-24<asteroida[i].y+64 && statek[nr_gracza].y-24>asteroida[i].y)) {
			kolizja=1;  //statek z leewj u dolu
			break; }
		if ((statek[nr_gracza].x-24<asteroida[i].x+64 && statek[nr_gracza].x-24>asteroida[i].x) && (statek[nr_gracza].y+24<asteroida[i].y+64 && statek[nr_gracza].y+24>asteroida[i].y)) {
			kolizja=1;  //statek z prawej u gory
			break; }
	}
		if (pkt50==1) {
		if ((pkt50_x+20>statek[nr_gracza].x-24 && pkt50_x+20<statek[nr_gracza].x+24) && (pkt50_y+20>statek[nr_gracza].y-24 && pkt50_y+20<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=1;  //statek z prawej u dolu
			}
		if ((pkt50_x<statek[nr_gracza].x+24 && pkt50_x>statek[nr_gracza].x-24) && (pkt50_y>statek[nr_gracza].y-24 && pkt50_y<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=1;  //statek z lewej u gory
			}
		if ((pkt50_x<statek[nr_gracza].x+24 && pkt50_x>statek[nr_gracza].x-24) && (pkt50_y+20>statek[nr_gracza].y-24 && pkt50_y+20<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=1;  //statek z leewj u dolu
			}
		if ((pkt50_x+20>statek[nr_gracza].x-24 && pkt50_x+20<statek[nr_gracza].x+24) && (pkt50_y>statek[nr_gracza].y-24 && pkt50_y<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=1;  //statek z prawej u gory
		}}

		if (bron_znajdzka.czy_jest==1) {
			if ((bron_znajdzka.x+20>statek[nr_gracza].x-24 && bron_znajdzka.x+20<statek[nr_gracza].x+24) && (bron_znajdzka.y+20>statek[nr_gracza].y-24 && bron_znajdzka.y+20<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=2;  //statek z prawej u dolu
			}
		if ((bron_znajdzka.x<statek[nr_gracza].x+24 && bron_znajdzka.x>statek[nr_gracza].x+24) && (bron_znajdzka.y>statek[nr_gracza].y-24 && bron_znajdzka.y<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=2;  //statek z lewej u gory
			}
		if ((bron_znajdzka.x<statek[nr_gracza].x+24 && bron_znajdzka.x>statek[nr_gracza].x+24) && (bron_znajdzka.y+20>statek[nr_gracza].y-24 && bron_znajdzka.y+20<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=2;  //statek z leewj u dolu
			}
		if ((bron_znajdzka.x+20>statek[nr_gracza].x-24 && bron_znajdzka.x+20<statek[nr_gracza].x+24) && (bron_znajdzka.y>statek[nr_gracza].y-24 && bron_znajdzka.y<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=2;  //statek z prawej u gory
		}}

		if (bomba.znajdzka==true) {
			if ((bomba.x+20>statek[nr_gracza].x-24 && bomba.x+20<statek[nr_gracza].x+24) && (bomba.y+20>statek[nr_gracza].y-24 && bomba.y+20<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=10;  //statek z prawej u dolu
			}
		if ((bomba.x<statek[nr_gracza].x+24 && bomba.x>statek[nr_gracza].x-24) && (bomba.y>statek[nr_gracza].y-24 && bomba.y<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=10;  //statek z lewej u gory
			}
		if ((bomba.x<statek[nr_gracza].x+24 && bomba.x>statek[nr_gracza].x-24) && (bomba.y+20>statek[nr_gracza].y-24 && bomba.y+20<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=10;  //statek z lewej u dolu
			}
		if ((bomba.x+20>statek[nr_gracza].x-24 && bomba.x+20<statek[nr_gracza].x+24) && (bomba.y>statek[nr_gracza].y-24 && bomba.y<statek[nr_gracza].y+24)) {
			kolizja_ze_znajdzka=10;  //statek z prawej u gory
		}}


	if (kolizja==1) return 1;					//gdy statek zderzy sie z asteroida
	if (kolizja_ze_znajdzka==1) return 50;		//gdy gracz zlapie "znajdzke"
	if (kolizja_ze_znajdzka==2) {				
		if (statek[nr_gracza].rodzaj_broni==2) bron=load_bmp("obrazy/czerwona_bron.bmp",default_palette);
		if (statek[nr_gracza].rodzaj_broni==1) bron=load_bmp("obrazy/zielona_bron.bmp",default_palette);
		if (!bron) {
			allegro_message("Wystapil blad z zaladowaniem bitmapy!");
			return 0;
		}
		return 2;
	}

	if (kolizja_ze_znajdzka==10) return 10;
	else return 0;
}

//===========================Tablica wynikow=========================
int tablica_wynikow (){
	int j;
			//Otwarcie pliku
	plik.open("tablica_wynikow.txt",ios::in);
	if( plik.is_open() == false ) { 
		allegro_message("Nie udalo sie otworzyc pliku z najlepszymi wynikami"); 
		return 0; 
	}
	else { //gdy uda sie otworzyc plik to wykonuj...
	i=1;
	char tmp[30];
	while(!plik.eof() && i<=10) {
		sprintf_s(gracz[i], "%d", i);
		if (i<10) strcat_s(gracz[i],". ");
		if (i==10) strcat_s(gracz[i],".");
		plik.getline(tmp,27);		//Odczyt linii z pliku
		strcat_s(gracz[i],tmp);
		i++;
	}

	blit(tlo_tablicy_wynikow,bmp,0,0,0,0,800,600);
	for (j=1;j<i;j++) {	//Wypisywanie poszczegolnych wynikow
		textout_ex(bmp,r2014,gracz[j],150,200+(j*20),makecol(0,255,0),-1); }
	textout_ex(bmp,r2014,"Wciscnij ESC, zeby wyjsc do glownego menu",135,230+(i*20),makecol(0,255,0),-1);
	blit(bmp,screen,0,0,0,0,800,600);
	while(!key[KEY_ESC]);
	plik.close();
	}
return 0;
}

//===========================Dodawanie wyniku do tablicy wynikow=================
int dodaj_wynik_do_tablicy_wynikow (int wynik) {
	fstream plik;
	int wyniki[11],j;
	//char aktualny_gracz[30];
	char tmp[30],liczba[6];

	//strcpy_s(aktualny_gracz,nazwa_gracza);
	sprintf_s(tmp, "%d", wynik);
	wyniki[0]=wynik;
	//strcat_s(nazwa_gracza,tmp);
	strcpy_s(gracz[0],nazwa_gracza);
	strcat_s(gracz[0],tmp);
	//allegro_message(gracz[0]);
	plik.open("tablica_wynikow.txt", ios::in);		//--otwarcie pliku
	if( plik.is_open() == false ) 
		allegro_message("Nie udalo sie otworzyc pliku z najlepszymi wynikami");
	else {		//--gdy uda sie otworzyc plik to:
		i=1;
		char tmp[25];
		while(!plik.eof() && i<=10) {
		plik.getline(tmp,27);	//wczytywanie linii z pliku
		strcpy_s(gracz[i],tmp);
		for(j=0;j<6;j++) liczba[j]=tmp[j+19];
		liczba[6]='\0';
		wyniki[i]=atoi(liczba);
		//printf("Wynik: %d",wyniki[i]);
		//allegro_message(liczba);
		i++;
		}
		plik.close(); }
		for (i=0;i<=10;i++) {	//sortowanie wynikow
			for (j=0;j<=10;j++) {
				if (wyniki[j]<wyniki[j+1] && wyniki[j]>0 && wyniki[j+1]>0) {
					int pom=wyniki[j+1];
					char pom_str[30];
					strcpy_s(pom_str,gracz[j+1]);
					wyniki[j+1]=wyniki[j];
					wyniki[j]=pom;
					strcpy_s(gracz[j+1],gracz[j]);
					strcpy_s(gracz[j],pom_str);
				}}}
		//--Otwarcie pliku do zapisu
		plik.open("tablica_wynikow.txt", ios::out );
		if (plik.is_open()==false) {
			allegro_message("Nie udalo sie otworzyc pliku do zapisania tablicy wynikow");
			return 0; 
			}
		else {
		for (i=1;i<=10;i++) { //zapis do pliku
			plik<<gracz[i]<<endl;
			if (plik.fail()) {
				allegro_message("Nieudany zapis do pliku");
				return 0; }
			} 
		plik.close();
		strcpy_s(nazwa_gracza,"Gracz               ");		//Podstawienie po nazwe gracza tej nazwy by uniknac bledow
		//allegro_message("Plik zamkniety");
		}	
	//}
return 0;
}

//===========================Glowna procedura gry=========================
int gra (void) {
	install_timer();
	install_int_ex( increment_speed, BPS_TO_TIMER( 60 ) ); //ilosc klatek na sekunde
	speed=0;
	int pozycja_napisu_z_poziomem=-45;
	int polozenie_tla; int polozenie_tla2; int polozenie_gwiazd; int polozenie_gwiazd2; int polozenie_gwiazdy_smierci;
	int polozenie_gwiazd3; int polozenie_gwiazd4;
	int petla=0,czy_kolizja,j,los;
	char str_punkty[10][4],str_ile_pozostalo[10],str_poziom_gry[10];
	int przezroczystosc_eksplozji,przezroczystosc_napisu=0;
	char napis_z_poziomem[20];
	bool przyspieszenie=false; bool przyspieszenie2=false;
	bool prawo=false; bool lewo=false;

	if (nowa_gra==1) {		//Gdy gracz rozpoczyna nowa gre
		statek[nr_gracza].rodzaj_broni=1;
		bron=load_bmp("obrazy/czerwona_bron.bmp",default_palette);
		punkty[nr_gracza]=0;
		poziom_gry=1;
		ilosc_asteroid=10;
		bomba.ilosc=1;
		bomba.znajdzka=false;
		bomba.uzyto=false;
		pkt50=0;
		bron_znajdzka.czy_jest=0;
		polozenie_tla=-600;
		polozenie_tla2=-1800;
		polozenie_gwiazdy_smierci=-600;
		polozenie_gwiazd=-600;
		polozenie_gwiazd2=-1800;
		polozenie_gwiazd3=-600;
		polozenie_gwiazd4=-1800;
		if (nr_gracza==0) {//jeœli nie odebrano danych z serwera
			statek[nr_gracza].x=400; statek[nr_gracza].y=550; }
		if (nowa_gra==1 && poziom_trudnosci==2) ilosc_asteroid+=10;
		if (dzwiek_wlaczony==true) play_midi(muzyczka,1);	//odtworz muzyczke w tle
	}

	napis=load_bmp("obrazy/napis.bmp",default_palette);
	_itoa_s(poziom_gry,str_poziom_gry,10);
	strcpy_s(napis_z_poziomem,"Poziom gry: ");
	strcat_s(napis_z_poziomem,str_poziom_gry);
	textout_ex(napis,cyberspace,napis_z_poziomem,0,0,makecol(0,255,0),-1);

	cout<<"Poziom gry: "<<poziom_gry<<endl;
	if (nr_gracza==0) {	//jeœli nie odebrano danych z serwera
	for(i=0;i<ilosc_asteroid;i++) {
		los=(rand()%100)+1;
		if (poziom_trudnosci==1) {	//gdy ustawiono latwy poziom
			if (los<=20) {//20% szansy na wylosowanie twardszych asteroid
				destroy_bitmap(asteroida[i].bmp);
				asteroida[i].bmp=load_bmp("obrazy/asteroida2.bmp",default_palette);
				asteroida[i].lvl=2;
				asteroida[i].wybuch=0;
				}
			if (los>20 && los<=25) {//5% szansy na wylosowanie najtwardszych asteroid
				destroy_bitmap(asteroida[i].bmp);
				asteroida[i].bmp=load_bmp("obrazy/asteroida3.bmp",default_palette);
				asteroida[i].lvl=3;
				asteroida[i].wybuch=0;
				}
			if (los>25) {
			destroy_bitmap(asteroida[i].bmp);
			asteroida[i].bmp=load_bmp("obrazy/asteroida.bmp",default_palette);
			asteroida[i].lvl=1;
			asteroida[i].wybuch=0;
			}}
		if (poziom_trudnosci==2) {	//gdy ustawiono trudny poziom
			if (los<=50) {//50% szansy na wylosowanie twardszych asteroid na trudniejszym poziomie
				destroy_bitmap(asteroida[i].bmp);
				asteroida[i].bmp=load_bmp("obrazy/asteroida2.bmp",default_palette);
				asteroida[i].lvl=2;
				asteroida[i].wybuch=0;
				}
			if (los>50 && los<=60) {//10% szansy na wylosowanie najtwardszych asteroid
				destroy_bitmap(asteroida[i].bmp);
				asteroida[i].bmp=load_bmp("obrazy/asteroida3.bmp",default_palette);
				asteroida[i].lvl=3;
				asteroida[i].wybuch=0;
				}
			if (los>60) {	//40% szans na wylosowanie zwyklych asteroid
				destroy_bitmap(asteroida[i].bmp);
			asteroida[i].bmp=load_bmp("obrazy/asteroida.bmp",default_palette);
			asteroida[i].lvl=1;
			asteroida[i].wybuch=0;
			}}
		if (!asteroida[i].bmp) {
			allegro_message("Wystapil blad podczas wczytywania %d asteroidy",i);
			return 0; }
	}
	}

	ile_pozostalo=ilosc_asteroid;
	bomba.uzyto=false;
	przezroczystosc_eksplozji=255;
	//punkty=0;
	for(i=0;i<20;i++) {  //"zerowanie pociskow"
		pocisk[nr_gracza][i].strzelono=0;
		pocisk[nr_gracza][i].y=-10; 
	}
	
	srand(time(NULL));	   
	if (nr_gracza==0) 
	for(i=0;i<ilosc_asteroid;i++) {		//losowanie danych asteroid(wspolrzedne)
		asteroida[i].x=rand()%500+10;
		asteroida[i].y=rand()%50-150;
		if (poziom_trudnosci==1) asteroida[i].ry=rand()%3+1;
		else asteroida[i].ry=rand()%5+1;
		asteroida[i].rx=rand()%5-2;
		if (asteroida[i].lvl==1) asteroida[i].hp=10;	//podstawianie hp do asteroid w zaleznosci od ich poziomu
		if (asteroida[i].lvl==2) asteroida[i].hp=20;	
		if (asteroida[i].lvl==3) asteroida[i].hp=30;	
		asteroida[i].punkty=asteroida[i].hp;			//ilosc punktow za zestrzelenie asteroidy
		if (poziom_trudnosci==2) asteroida[i].punkty*=2;
		if (i>4) asteroida[i].y=asteroida[i].y-50*i; }
	
	WSAStartup();
	
	do  //------------------------glowna petla---------------------------------
	{
	while (speed>0) {		//speed - od timera - utrzymuje stala predkosc

	blit(tlo,bmp,0,0,0,polozenie_tla,800,1200);	//"wklejanie" bitmap na ekran
	blit(tlo,bmp,0,0,0,polozenie_tla2,800,1200);
	masked_blit(gwiazda_smierci,bmp,0,0,0,polozenie_gwiazdy_smierci,800,1200);
	masked_blit(gwiazdy,bmp,0,0,0,polozenie_gwiazd,800,1200);
	masked_blit(gwiazdy2,bmp,0,0,0,polozenie_gwiazd2,800,1200);
	masked_blit(gwiazdy3,bmp,0,0,0,polozenie_gwiazd3,800,1200);
	masked_blit(gwiazdy4,bmp,0,0,0,polozenie_gwiazd4,800,1200);
	if (pozycja_napisu_z_poziomem <150) {
		if (przezroczystosc_napisu<255 && petla%1==0) przezroczystosc_napisu++;
		set_trans_blender( 0, 0, 0, przezroczystosc_napisu);
		draw_trans_sprite( bmp, napis, 120, pozycja_napisu_z_poziomem );
		pozycja_napisu_z_poziomem++; }
	else {
		if (przezroczystosc_napisu>=0) {
			if (petla%1==0) przezroczystosc_napisu--;
			set_trans_blender( 0, 0, 0, przezroczystosc_napisu);
			draw_trans_sprite( bmp, napis, 120, pozycja_napisu_z_poziomem );}}
	if (petla%5==0) {polozenie_tla+=1; polozenie_tla2+=1;}	//przesuwanie bitmap w dol
	if (petla%2==0) polozenie_gwiazdy_smierci++;
	polozenie_gwiazd+=3; 
	polozenie_gwiazd2+=3;
	polozenie_gwiazd3+=6; 
	polozenie_gwiazd4+=6;
	if (polozenie_gwiazdy_smierci>600) polozenie_gwiazdy_smierci=-1200;
	if (polozenie_tla>=600) polozenie_tla=-1800;
	if (polozenie_tla2>=600) polozenie_tla2=-1800;
	if(polozenie_gwiazd>600) polozenie_gwiazd=-1800;
	if(polozenie_gwiazd2>600) polozenie_gwiazd2=-1800;
	if(polozenie_gwiazd3>600) polozenie_gwiazd3=-1800;
	if(polozenie_gwiazd4>600) polozenie_gwiazd4=-1800;

	for(i=0;i<ilosc_asteroid;i++) { 
		if (asteroida[i].hp<=0) continue;
		if(asteroida[i].x<=0||asteroida[i].x>=534) asteroida[i].rx=asteroida[i].rx*-1; //kolizja asteroidy ze sciana
		if(asteroida[i].y>600) asteroida[i].y=-64;		//teleportacja asteroidy
		asteroida[i].x=asteroida[i].x+asteroida[i].rx;	//przesuwanie asteroidy
		asteroida[i].y=asteroida[i].y+asteroida[i].ry; 
	}
	for (i=0;i<ilosc_asteroid;i++) {
		for (j=0;j<ilosc_asteroid;j++) {
		if (i==j) continue;
		else
			kolizja_asteroid2(i,j);		//kolizja pomiedzy asteroidami
		}}
	if (pkt50==1) pkt50_y+=2;			//przesuwanie "znajdziek"
	if (bron_znajdzka.czy_jest==1) bron_znajdzka.y+=2;
	if (bomba.znajdzka==true) bomba.y+=2;
	if (pkt50_y>630) pkt50=0;
	if (bron_znajdzka.y>630) bron_znajdzka.czy_jest=0;
	if (bomba.y>630) bomba.znajdzka=false;

	czy_kolizja=kolizja_statku();     //kolizja statku z innymi obiektami  
	if (czy_kolizja==1) {
		statek[nr_gracza].x=700;
		//return 0;
	}
	if (czy_kolizja==50) {
		punkty[nr_gracza]+=50;
		pkt50=0; }
	if (czy_kolizja==2) {
		bool sprawdzono=false;
		if (statek[nr_gracza].rodzaj_broni==1 && sprawdzono==false) {
			statek[nr_gracza].rodzaj_broni=2;
			punkty[nr_gracza]+=20;
			sprawdzono=true;}
		if (statek[nr_gracza].rodzaj_broni==2 && sprawdzono==false) {
			statek[nr_gracza].rodzaj_broni=1;
			punkty[nr_gracza]+=20;
			sprawdzono=true;}
		bron_znajdzka.czy_jest=0;
	}
	if (czy_kolizja==10) {
		punkty[nr_gracza]+=20;
		bomba.znajdzka=false;
		bomba.ilosc++;
	}

	prawo=false;
	lewo=false;

    //reagowanie na klawisze
	if(key[KEY_Q]) {
		dzwiek_wlaczony=true;
		play_midi(muzyczka,0);}
	if(key[KEY_W]) {
		dzwiek_wlaczony=false;
		stop_midi();}
	if(key[KEY_X]) {	//w³¹czanie/wy³¹czenie strzelania
		if(strzelanie) strzelanie=false;
		else strzelanie=true;}

	if (statek[nr_gracza].x<650) {
		if((key[KEY_LEFT])&&(statek[nr_gracza].x>24)) {
			statek[nr_gracza].x-=3; 
			lewo=true; }
		if((key[KEY_RIGHT])&&(statek[nr_gracza].x<576)) {
			statek[nr_gracza].x+=3;
			prawo=true; }
		if((key[KEY_UP])&&(statek[nr_gracza].y>24)) {
			statek[nr_gracza].y-=3; }
		if((key[KEY_DOWN])&&(statek[nr_gracza].y<576)) {
			statek[nr_gracza].y+=3; }
	}
	if(key[KEY_ESC]) {
		int przycisk;
		speed=0;
		przycisk=alert("Czy chcesz wyjsc z gry?","T/ENTER - Tak","N/ESC - Nie","Tak","Nie",84,78); 
		if (przycisk==1) return 0; 
		speed=0;
		//if (przycisk==2) {kl=0; return 5;}
	}

		strzal(petla);	//strzelanie

	for (int j=0;j<=3;j++) {
	for(i=0;i<20;i++) {  //rysowanie pociskow
	if (pocisk[j][i].strzelono==1) pocisk[j][i].y=pocisk[j][i].y-4;
	if (pocisk[j][i].y<-10) pocisk[j][i].strzelono=0;
	int czerwony=0,zielony=0,niebieski=0;
	switch (j) {
		case 1: niebieski=255;
			break;
		case 2: czerwony=255;
			break;
		case 3: zielony=255;
			break;
		default: {
			zielony=255;
			czerwony=255;
			niebieski=255;
				 }
	}
	if (statek[j].rodzaj_broni==1 && statek[j].x<700) rectfill(bmp,pocisk[j][i].x-1,pocisk[j][i].y-5,pocisk[j][i].x+1,pocisk[j][i].y+5,makecol(czerwony,zielony,niebieski)); 
	if (statek[j].rodzaj_broni==2 && statek[j].x<700) rectfill(bmp,pocisk[j][i].x-2,pocisk[j][i].y-2,pocisk[j][i].x+2,pocisk[j][i].y+2,makecol(255,0,0)); 
	//else rectfill(bmp,pocisk[j][i].x-1,pocisk[j][i].y-5,pocisk[j][i].x+1,pocisk[j][i].y+5,makecol(0,255,0));
	}
	}
	if (bomba.uzyto==true && przezroczystosc_eksplozji>0) {
		przezroczystosc_eksplozji--;
		set_trans_blender( 0, 0, 0, przezroczystosc_eksplozji);
		draw_trans_sprite( bmp, eksplozja, 0, 0 );}
	if(!prawo && !lewo && nr_gracza==0) masked_blit(s,bmp,0,0,statek[nr_gracza].x-24,statek[nr_gracza].y-24,48,48);	//"wklejanie" bitmapy statku
	if (prawo && !lewo && nr_gracza==0) masked_blit(s_prawo,bmp,0,0,statek[nr_gracza].x-24,statek[nr_gracza].y-24,48,48);		//bitmapa statku pochylona w prawo
	if (lewo && !prawo && nr_gracza==0) masked_blit(s_lewo,bmp,0,0,statek[nr_gracza].x-24,statek[nr_gracza].y-24,48,48);		//--||--w lewo

		for (i=1;i<=3;i++) {
		if (nr_gracza != 0 && statek[i].x<700) {//gdy gracz jest po³¹czony z serwerem i nie zostal zestrzelony
		masked_blit(statek[i].bmp,bmp,0,0,statek[i].x-24,statek[i].y-24,48,48);
		}
		}

	set_trans_blender( 0, 0, 0, 100);
	draw_trans_sprite( bmp, panel, 600, 0 );
	if (pkt50==1) masked_blit(plus_50,bmp,0,0,pkt50_x,pkt50_y,20,20);
	if (bron_znajdzka.czy_jest==1) masked_blit(bron,bmp,0,0,bron_znajdzka.x,bron_znajdzka.y,20,20);
	if (bomba.znajdzka==true) masked_blit(bomba_bmp,bmp,0,0,bomba.x,bomba.y,20,20);
	for(i=0;i<ilosc_asteroid;i++) {
		if (asteroida[i].wybuch>20) continue;
		masked_blit(asteroida[i].bmp,bmp,0,0,asteroida[i].x,asteroida[i].y,64,64); }
			
		for (i=0;i<ilosc_asteroid;i++) {
		if (asteroida[i].wybuch>0  && asteroida[i].x>-100) {
			set_trans_blender( 0, 0, 0, (asteroida[i].wybuch*8));
			draw_trans_sprite( bmp, wybuch, asteroida[i].x, asteroida[i].y );}
		}
		//"panel" z prawej strony ekranu
		vline(bmp,600,0,600,makecol(0,255,0));
		textout_ex(bmp, r2014, "Punkty", 610, 30,makecol(0, 255, 0), -1);
		_itoa(punkty[nr_gracza],str_punkty[nr_gracza],10);
		textout_ex(bmp, r2014, str_punkty[nr_gracza], 610, 50,makecol(0, 255, 0), -1);
		_itoa(ile_pozostalo,str_ile_pozostalo,10);

		for (i=i;i<=2;i++) {
		if (statek[i].x>600) {	//zamiast wysylac komunikaty, serwer "teleportuje" gracza - wtedy wiadomo, ze zakonczyl gre
			statek[i].zestrzelono=true;
			//textout_ex(bmp,font,"gracz %d zostal zniszczony!",620,500+i*15,makecol(255,0,0),-1);
		}
	}
		
		for (i = 1;i <=3; i++) {
			_itoa(punkty[i],str_punkty[i],10);
			int czerwony=0,zielony=0,niebieski=0;
			switch (i) {
				case 1 : niebieski=255;
				break;
				case 2 : czerwony=255;
				break;
				case 3 : zielony=255;
				break;
				default : niebieski=255;
					zielony=255;
					czerwony=255;
			}
			if (statek[i].x<700) textout_ex(bmp, r2014, str_punkty[i], statek[i].x+20, statek[i].y-30,makecol(czerwony, zielony, niebieski), -1);
			else if (statek[i].x>=700) textout_ex(bmp,r2014,"Gracz zniszczony!",600,400+(i*15),makecol(czerwony,zielony,niebieski),-1);
		}
		if (statek[1].x>=700 && statek[2].x>=700 && statek[3].x>=700) {
			allegro_message("Wszyscy gracze zostali zniszczeni!");
			return -1;
		}
		
		if (ile_pozostalo>4) {
		textout_ex(bmp, r2014, "Pozostalo", 610, 100,makecol(0, 255, 0), -1);
		textout_ex(bmp, r2014, str_ile_pozostalo, 610, 120,makecol(0, 255, 0), -1);
		textout_ex(bmp, r2014, "asteroid", 610, 140,makecol(0, 255, 0), -1);
		}
		if (ile_pozostalo>1 && ile_pozostalo <=4) {
			if (!przyspieszenie) {
			for (i=0;i<ilosc_asteroid;i++) asteroida[i].ry++;
			przyspieszenie=true; }
		textout_ex(bmp, r2014, "Pozostaly", 610, 100,makecol(255, petla*2, 0), -1);
		textout_ex(bmp, r2014, str_ile_pozostalo, 610, 120,makecol(255, petla*2, 0), -1);
		textout_ex(bmp, r2014, "asteroidy", 610, 140,makecol(255, petla*2, 0), -1);
		}
		if (ile_pozostalo<=1) {
			if (!przyspieszenie2) {
			for (i=0;i<ilosc_asteroid;i++) asteroida[i].ry++;
			przyspieszenie2=true; }
		textout_ex(bmp, r2014, "Pozostala", 610, 100,makecol(255, petla*5, 0), -1);
		textout_ex(bmp, r2014, str_ile_pozostalo, 610, 120,makecol(255, petla*5, 0), -1);
		textout_ex(bmp, r2014, "asteroida", 610, 140,makecol(255, petla*5, 0), -1);
		}
		
		_itoa_s(poziom_gry,str_poziom_gry,10);
		textout_ex(bmp, r2014, "Poziom gry", 610, 200, makecol(poziom_gry*10,255-(poziom_gry*10),0),-1);
		textout_ex(bmp, r2014, str_poziom_gry, 610, 220, makecol(poziom_gry*10,255-(poziom_gry*10),0),-1);
		if (bomba.uzyto==true || bomba.ilosc<=0) {
			textout_ex(bmp, r2014, "Bomby:", 610, 280, makecol(255,255-petla*5,0),-1);
			for (i=0;i<bomba.ilosc;i++) rectfill(bmp,610+i*20,300,630+i*20,320,makecol(255,255-petla*5,0));
		}		
		else textout_ex(bmp, r2014, "Bomby:", 610, 280, makecol(0,255,0),-1);
		for (i=0;i<bomba.ilosc;i++) {
			masked_blit(bomba_bmp,bmp,0,0,610+i*20,300,20,20);
		}
		textout_ex(bmp, r2014, "X - strzelanie", 610, 480, makecol(50,150,50),-1);
		textout_ex(bmp, r2014, "Dzwiek:", 610, 500, makecol(50,150,50),-1);
		textout_ex(bmp, r2014, "Q - wlaczony", 610, 520, makecol(50,150,50),-1);
		textout_ex(bmp, r2014, "W - wylaczony", 610, 540, makecol(50,150,50),-1);
			
		if (ilosc_nieudanych_polaczen>=5) {
			masked_blit(stracono_polaczenie,bmp,0,0,300,50,64,64);
			textout_ex(bmp,font,"Problem z polaczeniem",260,100,makecol(255,0,0),-1);
		}
		if (ilosc_nieudanych_polaczen>25) {
			allegro_message("Stracono polaczenie z serwerem!\nSerwer nie odpowiada");
			return 0;
		}
	blit(bmp,screen,0,0,0,0,800,600);
	//rest(10);
	petla++;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*wysylanie i odbieranie danych poszczegolnych graczy 20 razy na sekunde (60fps div 3)
	gracz1 = petla 0,3,6...
	gracz2 = petla 1,4,7...
	gracz3...
	*/
	int mod=petla%3;
	if (nr_gracza==1 && mod==0) UDP();
	else if (nr_gracza==2 && mod==1) UDP();
	else if (nr_gracza==3 && mod==2) UDP();
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (ile_pozostalo<=0  && bron_znajdzka.czy_jest==0 && pkt50==0 && bomba.znajdzka==false) {		//gdy wszystkie asteroidy zostaly zestrzelone
		poziom_gry++;
		if (poziom_gry<10 && ilosc_asteroid<50) ilosc_asteroid+=5;
		return poziom_gry;
	}
	speed--; 
	}}
	while (!key[KEY_BACKSPACE]);	//wykonuj petle, dopoki sa jeszcze asteroidy do zestrzelenia
		remove_int( increment_speed );
				stop_sample(laser);
				stop_midi();
	return 0;
}

//===========================Wczytanie nazwy gracza============================
void wczytaj_nazwe_uzytkownika (void) {
	char znak=0;
	int i,j=0;
	for (i=0;i<20;i++) {
		if (nazwa_gracza[i]==' ' && nazwa_gracza[i+1]==' ') break;
	}
			blit(tlo,bmp,0,0,0,0,800,600);
			textout_ex(bmp,cyberspace,"Wpisz nazwe gracza:",100,150,makecol(0,255,0),-1);
			textout_ex(bmp,cyberspace,nazwa_gracza,50,250,makecol(0,255,0),-1);
			blit(bmp,screen,0,0,0,0,800,600);
	while (znak!=13) {	//enter
		blit(tlo,bmp,0,0,0,0,800,600);
		//rectfill(bmp,50+i*31,300,81+i*31,305,makecol(0,255,0));
	znak=readkey();
	if (dzwiek_wlaczony==true) play_sample(menu_zmiana,255,127,1000,0);
	if (znak==8) {	//8 - backspace
		nazwa_gracza[i-1]=0;
		i-=2;
	}
	if (znak>=32 && znak<127) nazwa_gracza[i]=znak; //32-127 zakres znakow tekstowych w ASCII
	textout_ex(bmp,cyberspace,"Wpisz nazwe gracza:",100,150,makecol(0,255,0),-1);
	textout_ex(bmp,cyberspace,nazwa_gracza,50,250,makecol(0,255,0),-1);
	blit(bmp,screen,0,0,0,0,800,600);
	i++;
	}
	nazwa_gracza[i-1]=0; //usuwanie znaku entera;
	for (j=i-1;j<30;j++) {
		nazwa_gracza[j]=32;	//spacja
	}
	for (i=20;i<30;i++) {
		nazwa_gracza[i]=0; }
	allegro_message("Zapisano nazwe gracza jako: %s",nazwa_gracza);
}

//===================================Menu glowne======================
int menu (void) {
	install_timer();
	install_int_ex( increment_speed, BPS_TO_TIMER( 15 ) ); //ilosc klatek na sekunde
	speed=0;
	clear_keybuf();
	int petla=0, kod, menu_item=0, polozenie_tla=-600, polozenie_tla2=-1800; int polozenie_gwiazd=-600; int polozenie_gwiazd2=-1800; int polozenie_gwiazdy_smierci=-600;
	int pozycja_napisow=605, pozycja_tytulu=-50;
	char item[7][40];
	char poz_tr[10];
	bool wcisnieto=false;
	int x_napisow=100;
	bool x_napisow_rosnace=true;

	//wpisywanie tekstu do zmiennych char[], ktore sa elementami menu glownego
	if (poziom_trudnosci==1) strcpy_s(poz_tr,"Latwy");
	if (poziom_trudnosci==2) strcpy_s(poz_tr,"Trudny");
	strcpy_s(item[0],"Rozpocznij gre"); strcpy_s(item[1],"Poziom trudnosci: "); strcpy_s(item[2],"Tablica wynikow"); strcpy_s(item[3],"Nazwa gracza"); 
	strcpy_s(item[4],"Pomoc"); strcpy_s(item[5],"Dzwiek: "); strcpy_s(item[6],"Wyjscie");
	strcat_s(item[1],poz_tr);

	//if (dzwiek_wlaczony==true) play_midi(muzyczka,1);
			//play_sample(laser,155,127,50,1);

	while (kl!=KEY_ENTER) {
			clear_keybuf();
				while (speed>0) {
					petla++;
		//dodatkowe informacje przy elementach menu
		if (poziom_trudnosci==1) strcpy_s(poz_tr,"Latwy");
		if (poziom_trudnosci==2) strcpy_s(poz_tr,"Trudny");
		strcpy_s(item[1],"Poziom trudnosci: ");
		strcat_s(item[1],poz_tr);
		strcpy_s(item[3],"Nazwa gracza: ");
		strcat_s(item[3],nazwa_gracza);
			if (dzwiek_wlaczony==true) {
				strcpy_s(item[5],"Dzwiek: ");
				strcat_s(item[5],"wlaczony"); }
			if (dzwiek_wlaczony==false) {
				strcpy_s(item[5],"Dzwiek: ");
				strcat_s(item[5],"wylaczony"); }
		if (petla%4==0) {	//przesuwanie tla
			polozenie_tla++;
			polozenie_tla2++; }
		if (petla%2==0) {
			polozenie_gwiazdy_smierci++; }
				polozenie_gwiazd+=3; polozenie_gwiazd2+=3;
		if (polozenie_tla>600) polozenie_tla=-1800;		//"teleportowanie" bitmap po ich 
		if (polozenie_tla2>600) polozenie_tla2=-1800;	//calkowitym wyjechaniu poza ekran
		if (polozenie_gwiazdy_smierci>600) polozenie_gwiazdy_smierci=-1800;
		
		blit(tlo_menu,bmp,0,0,0,polozenie_tla,800,1200);//wklejanie bitmap na ekran
		blit(tlo_menu2,bmp,0,0,0,polozenie_tla2,800,1200);
		masked_blit(gwiazda_smierci,bmp,0,0,0,polozenie_gwiazdy_smierci,800,1200);
		masked_blit(gwiazdy,bmp,0,0,0,polozenie_gwiazd,800,1200);
		masked_blit(gwiazdy2,bmp,0,0,0,polozenie_gwiazd2,800,1200);
		    if (pozycja_napisow>475) pozycja_napisow-=2;
		blit(psk,bmp,0,0,100,pozycja_napisow,114,125);
			
			//Inforamcje u dolu
			textout_ex(bmp,r2014,"Politechnika Swietokrzyska w Kielcach",225,pozycja_napisow,makecol(50,150,50),-1);
			textout_ex(bmp,r2014,"Projekt z programowania w jezyku C",225,pozycja_napisow+25,makecol(50,150,50),-1);
			textout_ex(bmp,r2014,"Rok akademicki: 2012/2013",225,pozycja_napisow+50,makecol(50,150,50),-1);
			textout_ex(bmp,r2014,"Prowadzacy: dr inz. Robert Tomaszewski",225,pozycja_napisow+75,makecol(50,150,50),-1);
			textout_ex(bmp,r2014,"Wykonawcy: Bebenek Rafal i Bartosinski Adrian",225,pozycja_napisow+100,makecol(50,150,50),-1);

		if (polozenie_gwiazd>600) polozenie_gwiazd=-1800;
		if (polozenie_gwiazd2>600) polozenie_gwiazd2=-1800;
		//Napis Space shooter
		if (x_napisow_rosnace==true && petla%5==0) {
			x_napisow++;
			if (x_napisow>=120) x_napisow_rosnace=false; }
		if (x_napisow_rosnace==false && petla%5==0) {
			x_napisow--;
			if (x_napisow<=80) x_napisow_rosnace=true; }

		if (pozycja_tytulu<40) {
			pozycja_tytulu+=2;
			textout_ex(bmp,cyberspace,"Space shooter",200,pozycja_tytulu,makecol(0,petla*3,0),-1);
		}
		else
		textout_ex(bmp,cyberspace,"Space shooter",200,pozycja_tytulu,makecol(0,255,0),-1);

		for (i=0;i<7;i++) {			//wyswietlanie poszczegolnych elementow menu
		if (i!=menu_item) textout_ex(bmp, r2014, item[i], x_napisow, 150+(i*20),makecol(50, 100, 50), -1);
		if (i==menu_item) {			//Podswietlenie konkretnego elementu menu
		textout_ex(bmp, r2014, item[i], x_napisow+20, 150+(i*20),makecol(0, 255, 0), -1);
		if (i==1) textout_ex(bmp,r2014,"--ENTER--",x_napisow+400,170,makecol(0,255,0),-1);
		if (i==3) textout_ex(bmp,r2014,"--ENTER--",x_napisow+450,210,makecol(0,255,0),-1);
		if (i==5) textout_ex(bmp,r2014,"--ENTER--",x_napisow+400,250,makecol(0,255,0),-1);
		}}
		blit(bmp,screen,0,0,0,0,800,600);
		//reagowanie na wcisniecie klawiszy
		if (key[KEY_DOWN]) {
			menu_item++;
			if (dzwiek_wlaczony==true) play_sample(menu_zmiana,255,127,1000,0);
			if (menu_item>6) menu_item=0; 
			}
		if (key[KEY_UP]) {
			menu_item--;
			if (dzwiek_wlaczony==true) play_sample(menu_zmiana,255,127,1000,0);
			if (menu_item<0) menu_item=6;}
		//if (key[KEY_ESC]) return 0;
		if (key[KEY_ENTER]) {
			if(menu_item==0) {
					WSAStartup();
					cout<<"Zainnicjalizowano WinSock"<<endl;
					TCP();
					if (nr_gracza==2) s=load_bmp("obrazy/statek2.bmp",default_palette);
					else if (nr_gracza==3) s=load_bmp("obrazy/statek3.bmp",default_palette);
					else s=load_bmp("obrazy/statek.bmp",default_palette);
				kod=gra(); //wywolanie nowej gry
				nowa_gra=0;
				
				while(kod!=0) {	//gdy gracz przechodzi do nastepnego poziomu
					if (kod>1) { /*allegro_message("Gratulacje - przechodzisz do %d poziomu",poziom_gry);*/ 
						//Sleep(500);
						//TCP();
						kod=gra();}}
				if (kod==0) {	//koniec gry
					stop_sample(laser);
					allegro_message("Koniec Gry - ilosc zdobytych punktow: %d",punkty[nr_gracza]);
					if (punkty[nr_gracza]>0) dodaj_wynik_do_tablicy_wynikow(punkty[nr_gracza]);
					stop_midi();
					nowa_gra=1;
					speed=0;
					install_int_ex( increment_speed, BPS_TO_TIMER( 15 ) ); //ilosc klatek na sekunde
				}
			}
			if (menu_item==1) {	//zmiana poziomu trudnosci
				if (poziom_trudnosci==1) poziom_trudnosci=2;
				else poziom_trudnosci=1;
			}
			if (menu_item==2) {	//tablica najlepszych wynikow
				tablica_wynikow();
				speed=0; 
			}
			if (menu_item==3) {	//zmiana nazwy gracza
				wczytaj_nazwe_uzytkownika();
				speed=0;
			}
			if (menu_item==4) {	//pomoc do gry
				int ktora_strona=1; int petla=0;
				while(!key[KEY_ESC]) {
				petla++;
				if (ktora_strona==1) blit(pomoc,bmp,0,0,0,0,800,600);
				else blit(pomoc2,bmp,0,0,0,0,800,600);
				if (key[KEY_LEFT] || key[KEY_RIGHT]) {
					rest(100);
					if (ktora_strona==1) ktora_strona=2;
					else ktora_strona=1; }
				textout_ex(bmp,r2014,"Strzalka w lewo lub prawo - przelaczenie pomiedzy stronami pomocy",25,560,makecol(0,petla,0),-1);
				textout_ex(bmp,r2014,"ESC - wyjscie do glownego menu",200,580,makecol(0,petla,0),-1);
				blit(bmp,screen,0,0,0,0,800,600);
				}
				speed=0;
			}
			if (menu_item==5) {	//wlaczenie/wylaczenie dzieku w grze
				if (dzwiek_wlaczony==true) dzwiek_wlaczony=false;
				else dzwiek_wlaczony=true;
			}
			if(menu_item==6) return 0;} //wyjscie z gry
		//rest(5);
				speed--;}}
		remove_int( increment_speed );
	return 0;
}

//======================Program g³ówny==============================
int main(void)
{
	int kod_menu;
	nowa_gra=1;

	int kod_bledu=inicjalizacja(); //inicjalizacja i sprawdzenie czy wszystko w porzadku
	if (kod_bledu==0) {
		allegro_message("Wystapil blad podczas inicjalizacji\nProgram zostanie zamkniety\nSprawdz, czy wszystkie pliki znajduja sie w folderze z gra");
		allegro_exit();
		return 0; }
	
	//gra();
	kod_menu=menu();
	if (kod_menu==5) menu();
	cout<<"Usuwanie bitmap, czcionek i dzwiekow";
		//if (kod_menu==0) return 0;
	//Usuwanie bitmap, czcionek i dzwiekow, zeby uniknac "wyciekow pamieci"
	destroy_bitmap(bmp);
	destroy_bitmap(s);
	destroy_bitmap(tlo);
	destroy_bitmap(tlo2);
	destroy_bitmap(tlo_menu);
	destroy_bitmap(tlo_menu2);
	destroy_bitmap(gwiazda_smierci);
	destroy_bitmap(gwiazdy);
	destroy_bitmap(gwiazdy2);
	destroy_bitmap(gwiazdy3);
	destroy_bitmap(gwiazdy4);
	destroy_bitmap(pomoc);
	destroy_bitmap(pomoc2);
	destroy_bitmap(eksplozja);
	destroy_bitmap(wybuch);
	destroy_bitmap(napis);
	for (i=0;i<50;i++) destroy_bitmap(asteroida[i].bmp);
	destroy_font(cyberspace);
	destroy_font(r2014);
	destroy_midi(muzyczka);
	destroy_sample(laser);
	destroy_sample(laser2);
	destroy_sample(menu_zmiana);
	destroy_sample(wybuch_sample);
    allegro_exit();
	return 0;
}