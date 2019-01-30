#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
extern "C" {

#include"./sdl/include/SDL.h"
#include"./sdl/include/SDL_main.h"

}
#define stdin  (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))
#define save "gra.save"
#define Plansza_szerokosc 10
#define PLansza_wysokosc 24
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480


struct  double_pkt
{
	double x;
	double y;
};
/* Struktura do definiowania klocków*/
struct klocek {
	int id;
	int kolor_obrysu;
	int kolor_wypelnienia;
	SDL_Point Punkty[4];
};

/*Wszystkie zmienne opisuj¹ce gre jako struktura*/
struct Gra {
	int stan_planszy[PLansza_wysokosc][Plansza_szerokosc];
	klocek obecnie_opadajacy;
	double_pkt Punkt_opadajacego_klocka;
	int szybkosc_opadania_kloca;
	int rozmiar_klocka;
	int kat;
	bool pause;
	bool koniec;
	int etap;
	// Etap co 60 sekund *1.2 czas
	int punkty;
	bool czy_4_usunieto;
};
// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
//stackoverflow
void DrawText(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		//szybkie kopiowanie pikseli na docelow¹ powierzchniê graficzn¹
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};



// rysowanie pojedynczego pixela
//stackoverflow
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};

// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};
void rysuj_plansze(SDL_Surface *screen,int kolor_obrysu,int kolor_wypelnienia,int rozmiar_klocka)
{
	
	/*Pion*/
	for (int i = 0; i < PLansza_wysokosc+2; i++)
	{
		DrawRectangle(screen, 80, rozmiar_klocka * i + 40, rozmiar_klocka, rozmiar_klocka, kolor_obrysu, kolor_wypelnienia);
		DrawRectangle(screen, rozmiar_klocka * 11 + 80, rozmiar_klocka * i + 40, rozmiar_klocka, rozmiar_klocka, kolor_obrysu, kolor_wypelnienia);
	}
	/*Poziom*/
	for (int i = 1; i <= Plansza_szerokosc+1; i++)
	{
		DrawRectangle(screen, rozmiar_klocka * i+80, 40, rozmiar_klocka, rozmiar_klocka, kolor_obrysu, kolor_wypelnienia);
		DrawRectangle(screen, rozmiar_klocka * i+80, rozmiar_klocka * (PLansza_wysokosc+1) + 40, rozmiar_klocka, rozmiar_klocka, kolor_obrysu, kolor_wypelnienia);
	}
}
/*Rysowanie klocka który w³asnie opada*/
void rysuj_padajacy_klocek(Gra *stan, SDL_Surface * screen, int rozmiar_klocka)
{
	/*Klocek -> 4 kadraciki, kazdy ze struktóry klocka trzeba wyœwietliæ*/
	for (int i = 0; i < 4;i++)
		DrawRectangle(screen, 
		stan->Punkt_opadajacego_klocka.x + 80 + rozmiar_klocka*(1+ stan->obecnie_opadajacy.Punkty[i].x),
		stan->Punkt_opadajacego_klocka.y + 40 + rozmiar_klocka * stan->obecnie_opadajacy.Punkty[i].y,
		rozmiar_klocka, rozmiar_klocka, stan->obecnie_opadajacy.kolor_obrysu, stan->obecnie_opadajacy.kolor_wypelnienia);

	

}

/*Pocz¹tkowy stan gry*/
void restetuj(Gra *stan)
{
	stan->czy_4_usunieto = false;
	for (int i = 0; i < PLansza_wysokosc; i++)
	for (int j = 0; j < Plansza_szerokosc; j++)
		stan->stan_planszy[i][j] = -1;
	stan->punkty = 0;
	stan->kat = 0;
	stan->koniec = false;
	stan->etap = 1;
	stan->pause = false;
	int rozmiar_klocka = (SCREEN_HEIGHT > SCREEN_WIDTH) ? SCREEN_WIDTH : SCREEN_HEIGHT;
	rozmiar_klocka -= 40; // Nag³ówek + margines
	rozmiar_klocka /= (PLansza_wysokosc > Plansza_szerokosc) ? PLansza_wysokosc + 2 : Plansza_szerokosc + 2;
	stan->Punkt_opadajacego_klocka.x = -1; // - 1 klocek nie istniej 
	stan->szybkosc_opadania_kloca = rozmiar_klocka * 4;
	stan->rozmiar_klocka = rozmiar_klocka;
}

/*Pocz¹tkowy stan gry + ustawienie czasu */
void restetuj(Gra *stan, double &worldTime)
{
	worldTime = 0;
	restetuj(stan);
}

void definicja_klockow(klocek *rodzaje, SDL_Surface * screen)
{
	/*ka¿dy punkt to lewy gorny rog kazdego z 4 klockow*/
	rodzaje[0].id = 0;
	rodzaje[0].kolor_obrysu = SDL_MapRGB(screen->format, 0xEE, 0x00, 0x00);
	rodzaje[0].kolor_wypelnienia = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	rodzaje[0].Punkty[0].x = 0;
	rodzaje[0].Punkty[0].y = 0;
	rodzaje[0].Punkty[1].x = 0;
	rodzaje[0].Punkty[1].y = 1;
	rodzaje[0].Punkty[2].x = 0;
	rodzaje[0].Punkty[2].y = 2;
	rodzaje[0].Punkty[3].x = 0;
	rodzaje[0].Punkty[3].y = 3;

	rodzaje[1].id =1;
	rodzaje[1].kolor_obrysu = SDL_MapRGB(screen->format, 0xEE, 0xee, 0x00);
	rodzaje[1].kolor_wypelnienia = SDL_MapRGB(screen->format, 0xFF, 0xff, 0x00);
	rodzaje[1].Punkty[0].x = 0;
	rodzaje[1].Punkty[0].y = 0;
	rodzaje[1].Punkty[1].x = 1;
	rodzaje[1].Punkty[1].y = 1;
	rodzaje[1].Punkty[2].x = 2;
	rodzaje[1].Punkty[2].y = 0;
	rodzaje[1].Punkty[3].x = 1;
	rodzaje[1].Punkty[3].y = 0;

	rodzaje[2].id = 2;
	rodzaje[2].kolor_obrysu = SDL_MapRGB(screen->format, 0xEE, 0xee, 0x88);
	rodzaje[2].kolor_wypelnienia = SDL_MapRGB(screen->format, 0xFF, 0xff, 0xaa);
	rodzaje[2].Punkty[0].x = 0;
	rodzaje[2].Punkty[0].y = 0;
	rodzaje[2].Punkty[1].x = 1;
	rodzaje[2].Punkty[1].y = 0;
	rodzaje[2].Punkty[2].x = 0;
	rodzaje[2].Punkty[2].y = 1;
	rodzaje[2].Punkty[3].x = 1;
	rodzaje[2].Punkty[3].y = 1;



	rodzaje[3].id = 3;
	rodzaje[3].kolor_obrysu = SDL_MapRGB(screen->format, 0xEE, 0xee, 0x88);
	rodzaje[3].kolor_wypelnienia = SDL_MapRGB(screen->format, 0xFF, 0xff, 0xaa);
	rodzaje[3].Punkty[0].x = 0;
	rodzaje[3].Punkty[0].y = 0;
	rodzaje[3].Punkty[1].x = 0;
	rodzaje[3].Punkty[1].y = 1;
	rodzaje[3].Punkty[2].x = 0;
	rodzaje[3].Punkty[2].y = 2;
	rodzaje[3].Punkty[3].x = 1;
	rodzaje[3].Punkty[3].y = 2;


	rodzaje[4].id = 4;
	rodzaje[4].kolor_obrysu = SDL_MapRGB(screen->format, 0x22, 0xee, 0x88);
	rodzaje[4].kolor_wypelnienia = SDL_MapRGB(screen->format, 0x44, 0xff, 0xaa);
	rodzaje[4].Punkty[0].x = 0;
	rodzaje[4].Punkty[0].y = 0;
	rodzaje[4].Punkty[1].x = 0;
	rodzaje[4].Punkty[1].y = 1;
	rodzaje[4].Punkty[2].x = 0;
	rodzaje[4].Punkty[2].y = 2;
	rodzaje[4].Punkty[3].x = -1;
	rodzaje[4].Punkty[3].y = 2;

	rodzaje[5].id = 5;
	rodzaje[5].kolor_obrysu = SDL_MapRGB(screen->format, 0xEE, 0x22, 0x88);
	rodzaje[5].kolor_wypelnienia = SDL_MapRGB(screen->format, 0xFF, 0x44, 0xaa);
	rodzaje[5].Punkty[0].x = 0;
	rodzaje[5].Punkty[0].y = 0;
	rodzaje[5].Punkty[1].x = 0;
	rodzaje[5].Punkty[1].y = 1;
	rodzaje[5].Punkty[2].x = 1;
	rodzaje[5].Punkty[2].y = 0;
	rodzaje[5].Punkty[3].x = -1;
	rodzaje[5].Punkty[3].y = 1;

	rodzaje[6].id = 6;
	rodzaje[6].kolor_obrysu = SDL_MapRGB(screen->format, 0xEE, 0xee, 0x22);
	rodzaje[6].kolor_wypelnienia = SDL_MapRGB(screen->format, 0xFF, 0xff, 0x44);
	rodzaje[6].Punkty[0].x = 0;
	rodzaje[6].Punkty[0].y = 0;
	rodzaje[6].Punkty[1].x = 0;
	rodzaje[6].Punkty[1].y = 1;
	rodzaje[6].Punkty[2].x = -1;
	rodzaje[6].Punkty[2].y = 0;
	rodzaje[6].Punkty[3].x = 1;
	rodzaje[6].Punkty[3].y = 1;
}
/*Obrót klocka*/
void swap_x_y(Gra *stan,int i)
{

	int temp = stan->obecnie_opadajacy.Punkty[i].x;
	stan->obecnie_opadajacy.Punkty[i].x = stan->obecnie_opadajacy.Punkty[i].y;
	stan->obecnie_opadajacy.Punkty[i].y = temp;
}


int znajdz_najnieszy_pkt_klocka(klocek k)
{
	int max = k.Punkty[0].y;
	for (int i = 1; i < 4; i++)
		if (max < k.Punkty[i].y)
			max = k.Punkty[i].y;

	return max;
}
int znajdz_najbardziej_na_lewo(klocek k)
{
	int max = k.Punkty[0].x;
	for (int i = 1; i < 4; i++)
	if (max > k.Punkty[i].x)
		max = k.Punkty[i].x;

	return max;
}

int znajdz_najbardziej_na_prawo(klocek k)
{
	int max = k.Punkty[0].x;
	for (int i = 1; i < 4; i++)
		if (max < k.Punkty[i].x)
			max = k.Punkty[i].x;

	return max;
}

//dodanie klocka do gry
void dodaj_klocek_do_stanu(Gra *stan,int x,int y)
{
	if (stan->Punkt_opadajacego_klocka.x != -1) //je¿eli klocek istnieje
	for (int i = 0; i < 4; i++)
	{
		stan->stan_planszy[y - 1 + stan->obecnie_opadajacy.Punkty[i].y][x + stan->obecnie_opadajacy.Punkty[i].x] = stan->obecnie_opadajacy.id;
	}
}

//rysowanie klocka umieszczonego na dole planszy
void rysuj_stan(Gra stan,klocek *rodzaje,SDL_Surface * screen)
{
	for (int i = 0; i < PLansza_wysokosc;i++)
		for (int j = 0; j < Plansza_szerokosc;j++)
			if (stan.stan_planszy[i][j]>=0)
				DrawRectangle(screen, (j+1)*stan.rozmiar_klocka+80, (i+1)*stan.rozmiar_klocka+40, 
					stan.rozmiar_klocka, stan.rozmiar_klocka, rodzaje[stan.stan_planszy[i][j]].kolor_obrysu, 
						rodzaje[stan.stan_planszy[i][j]].kolor_wypelnienia);
}
//stworzenie klocka jesli zaden nie opada -> dodanie go do gry
//jesli jakis opada to ustalanie szybkosci opadania i czy bedzie opadac na planszy, nie poza nia
//po dodaniu do gry, ustawienie ze zaden klocek nie opada
void opadanie_klocka_i_kolizje(Gra *stan,double delta,klocek *rodzaje,SDL_Surface *screen)
{
	
	if (stan->Punkt_opadajacego_klocka.x == -1) // jezeli klocek nie opdada
	{
		/*generowanie klocka*/
		stan->kat = 0;
		stan->Punkt_opadajacego_klocka.x = (rand() % (Plansza_szerokosc-4))*stan->rozmiar_klocka+2*stan->rozmiar_klocka;
		stan->Punkt_opadajacego_klocka.y = stan->rozmiar_klocka; 
		stan->obecnie_opadajacy = rodzaje[rand()%7]; 
	}
	else
	{
		stan->Punkt_opadajacego_klocka.y += delta*stan->szybkosc_opadania_kloca; // implementacja szybkosci opadania
		rysuj_padajacy_klocek(stan, screen, stan->rozmiar_klocka);
		for (int i = 0; i < 4; i++)
		{
			// sprawdzenie czy wchodzi lewa czescia 
			int y = (stan->obecnie_opadajacy.Punkty[i].y + 1) + stan->Punkt_opadajacego_klocka.y / stan->rozmiar_klocka;
			double x = stan->Punkt_opadajacego_klocka.x / stan->rozmiar_klocka + stan->obecnie_opadajacy.Punkty[i].x;
			if (stan->stan_planszy[y - 1][int(x)] >= 0 || y >= PLansza_wysokosc + 1)
			{
				dodaj_klocek_do_stanu(stan, stan->Punkt_opadajacego_klocka.x/stan->rozmiar_klocka, stan->Punkt_opadajacego_klocka.y / stan->rozmiar_klocka);

				stan->Punkt_opadajacego_klocka.x = -1;
				return;
			}
			//sprawdzenie czy prawa
			if (stan->stan_planszy[y - 1][int(x+0.5)] >= 0 || y >= PLansza_wysokosc + 1)
			{
				dodaj_klocek_do_stanu(stan, int(stan->Punkt_opadajacego_klocka.x / stan->rozmiar_klocka+0.5), stan->Punkt_opadajacego_klocka.y / stan->rozmiar_klocka);

				stan->Punkt_opadajacego_klocka.x = -1;
				return;
			}
		}
	}
}
/*Usuwanie pe³nych wierszy */
void kasuj_wiersze(Gra *stan)
{
	int ilosc = 0;
	for (int i = 0; i < PLansza_wysokosc; i++)
	{
		bool flag = true;
		for (int j = 0; j < Plansza_szerokosc; j++)
		{
			if (stan->stan_planszy[i][j] < 0)
				flag = false;
		}
		if (flag == true)
		{
			stan->punkty += 100*stan->etap;
			ilosc++;
			for (int k = 1; k < i; k++)
				for (int j = 0; j < Plansza_szerokosc; j++)
					stan->stan_planszy[i-k+1][j] = stan->stan_planszy[i - k][j];
		}
	}
	/*bonusy*/
	if (ilosc>2) stan->punkty += 100 * stan->etap;
	if (ilosc >= 4 && !stan->czy_4_usunieto) stan->czy_4_usunieto = true;
	if (ilosc <4)	stan->czy_4_usunieto = false;
	if (ilosc >= 4 && stan->czy_4_usunieto) stan->punkty += 400 * stan->etap;
	if (ilosc >= 4) stan->punkty += 400 * stan->etap;
}
/*Sprawdzenie czy moze sie przesun¹c poziomo w lewo*/
bool czy_klocek_moze_w_lewo(Gra stan)
{
	int x_podstawowy = stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka;
	int y_podstawowy = stan.Punkt_opadajacego_klocka.y / stan.rozmiar_klocka;
	for (int i = 0; i < 4; i++)
		/*Przy x -1 bo na lewo */
		if (stan.stan_planszy[y_podstawowy + stan.obecnie_opadajacy.Punkty[i].y][x_podstawowy - 1 + stan.obecnie_opadajacy.Punkty[i].x]>=0)
			return false;

	return true;
}

/*Sprawdzenie czy moze sie przesyn¹c poziomo w prawo*/
bool czy_klocek_moze_w_prawo(Gra stan)
{
	int x_podstawowy = stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka;
	int y_podstawowy = stan.Punkt_opadajacego_klocka.y / stan.rozmiar_klocka;
	for (int i = 0; i < 4; i++)
		/*Przy x  +1 bo to co na prawo */
		if (stan.stan_planszy[y_podstawowy + stan.obecnie_opadajacy.Punkty[i].y][x_podstawowy + 1 + stan.obecnie_opadajacy.Punkty[i].x] >= 0)
			return false;

	return true;
}
/*Definiowanie przegranej */
void czy_przegrana(Gra * stan)
{
	for (int i = 0; i<4;i++)
	for (int j = 0; j < Plansza_szerokosc;j++)
	if (stan->stan_planszy[i][j] >= 0)
	{
		stan->pause = true;
		stan->koniec = true;
	}
}
/*Spadek klocka (strza³ka w dó³)*/
void umiesc_na_dole(Gra *stan)
{

/*Wyszukanie najnizszej pozycji by umieœciæ klocek */
	if (stan->Punkt_opadajacego_klocka.x>=0)
	for (int j = stan->Punkt_opadajacego_klocka.y / stan->rozmiar_klocka; j <= PLansza_wysokosc; j++)
		for (int i = 0; i < 4; i++)
		{
		// sprawdzenie czy wchodzi lewa czesc
			int y = (stan->obecnie_opadajacy.Punkty[i].y +1) + j;
			double x = stan->Punkt_opadajacego_klocka.x / stan->rozmiar_klocka + stan->obecnie_opadajacy.Punkty[i].x;
			if (stan->stan_planszy[y - 1][int(x)] >= 0 || y >= PLansza_wysokosc + 1)
			{
				dodaj_klocek_do_stanu(stan, int(stan->Punkt_opadajacego_klocka.x / stan->rozmiar_klocka), j);

				stan->Punkt_opadajacego_klocka.x = -1;
				return;
			}
			//prawa
			if (stan->stan_planszy[y - 1][int(x + 0.5)] >= 0 || y >= PLansza_wysokosc + 1)
			{
				dodaj_klocek_do_stanu(stan, int(stan->Punkt_opadajacego_klocka.x / stan->rozmiar_klocka + 0.5), j);

				stan->Punkt_opadajacego_klocka.x = -1;
				break;
			}

		}
}

/*Etapowosæ*/
void etapy(Gra *stan, double worldTime)
{
	if (stan->etap == 1 & worldTime > 60)
	{
		stan->szybkosc_opadania_kloca *= 1.2;
		stan->etap = 2;
	}
	else if (stan->etap == 2 & worldTime > 60 * 2)
	{
		stan->szybkosc_opadania_kloca *= 1.2;
		stan->etap = 3;
	}
	else if (stan->etap == 3 & worldTime > 60 * 3)
	{
		stan->szybkosc_opadania_kloca *= 1.2;
		stan->etap = 4;
	}
	else if (stan->etap == 4 & worldTime > 60 * 4)
	{
		stan->szybkosc_opadania_kloca *= 1.2;
		stan->etap = 5;
	}
}
/* wed³ug  POINT[0] obraca pozosta³e */
void obrot(Gra *stan)
{
	klocek kopia = stan->obecnie_opadajacy;

	stan->kat += 1;
	for (int i = 0; i < 4; i++)
	{
		if (stan->kat % 2 == 1)
		{

			swap_x_y(stan, i);
			/*Nie przewracanie na sciane */
			if (stan->Punkt_opadajacego_klocka.x + znajdz_najbardziej_na_lewo(stan->obecnie_opadajacy)*stan->rozmiar_klocka < 0)
			{
				stan->obecnie_opadajacy = kopia;
				return;
			}

			if (stan->Punkt_opadajacego_klocka.x / stan->rozmiar_klocka >
				Plansza_szerokosc - znajdz_najbardziej_na_prawo(stan->obecnie_opadajacy) - 1)
			{
				stan->obecnie_opadajacy = kopia;
				return;

			}
		}
		else if (stan->kat % 2 == 0){
			swap_x_y(stan, i);
			/** -1 -> obracanie o 360 stopni z w³asnoœci trygonometrycznych  */
			stan->obecnie_opadajacy.Punkty[i].x *= -1;
			stan->obecnie_opadajacy.Punkty[i].y *= -1;
				/*Przyrwócenie w razie konfliktów*/
			if (stan->Punkt_opadajacego_klocka.x + znajdz_najbardziej_na_lewo(stan->obecnie_opadajacy)*stan->rozmiar_klocka < 0)
			{
				stan->obecnie_opadajacy = kopia;
				return;
			}
			if (stan->Punkt_opadajacego_klocka.x / stan->rozmiar_klocka >
				Plansza_szerokosc - znajdz_najbardziej_na_prawo(stan->obecnie_opadajacy) - 1)
			{
				stan->obecnie_opadajacy = kopia;
				return;

			}
			


		}
	}
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	srand(time(NULL));
	
	klocek rodzaje[7];
	Gra stan;
	restetuj(&stan);
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *eti;
	SDL_Texture *scrtex;
	//struktura trzymajaca wszystkie informacje oknie
	SDL_Window *window;
	//renderowanie, zwiazana z oknem
	SDL_Renderer *renderer;
	
	//inicjowanie biblioteki SDL
	//zwraca informacje o ostatnim b³êdzie który wyst¹pi³
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	//tworzenie i renderownie okna
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	//wskazanie jakoœci skalowania
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	//ustawienie rozdzielcozci rysowania
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	//kolory do renderowanie r g b a
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Tetris");

	//przydzielanie nowej powierchni RGB
	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	//utworzenie tekstury
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);


	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	//liczba milisekund od inicjalizacji SDL
	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	distance = 0;
	etiSpeed = 1;

	definicja_klockow(rodzaje, screen);



	while(!quit) {
		t2 = SDL_GetTicks();

		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplyna³ od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;
		
		SDL_FillRect(screen, NULL, czarny);


		/*implementacja pauzy */
		if (stan.pause == true)
		{
			
			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
			sprintf(text, "Tetris, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
			DrawText(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
			if (stan.koniec == true)
				sprintf(text, "Przegrales!!");
			
			else 
				sprintf(text, "PAUSE");

				DrawText(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
			
	
		}
		else {
			//prostokaty dookola planszy
			rysuj_plansze(screen, zielony, czerwony, stan.rozmiar_klocka);

			//linia graniczna
			DrawRectangle(screen,80, 40 + stan.rozmiar_klocka * 4 + (stan.rozmiar_klocka - 10), (Plansza_szerokosc + 2)*stan.rozmiar_klocka, 10, zielony, niebieski);
			rysuj_stan(stan, rodzaje, screen);
			kasuj_wiersze(&stan);

			opadanie_klocka_i_kolizje(&stan, delta, rodzaje, screen);
			//górny prostokat
			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
			sprintf(text, "Etap %d Punkty: %d czas trwania = %.1lf s  %.0lf klatek / s", stan.etap,stan.punkty ,worldTime, fps);
			DrawText(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
			sprintf(text, "Esc - wyjscie, \030 - obrot, \031 - spadek ,r -restart");
			DrawText(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
		}
	
		/*Etapowosc*/
		
		etapy(&stan, worldTime);

		/*PRzegrana*/
		czy_przegrana(&stan);


		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};

		
		//uaktualnienie danego prostokata na teksturze nowymi dnaymi pikselowymi
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);

		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_RIGHT 
							&& stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka <
							Plansza_szerokosc - znajdz_najbardziej_na_prawo(stan.obecnie_opadajacy) -1)
						{
						if (czy_klocek_moze_w_prawo(stan)) // sprawdzenie czy na prawo nie ma klocków
							stan.Punkt_opadajacego_klocka.x += delta*Plansza_szerokosc * 240;
						else
							stan.Punkt_opadajacego_klocka.x = int(stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka)*(stan.rozmiar_klocka);

							if (stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka >
								Plansza_szerokosc - znajdz_najbardziej_na_prawo(stan.obecnie_opadajacy) - 1)
							{ // implementacja gdyby wyszlo za ramke 
								stan.Punkt_opadajacego_klocka.x = (Plansza_szerokosc - znajdz_najbardziej_na_prawo(stan.obecnie_opadajacy) - 1)*stan.rozmiar_klocka;
							}
						}
						
					else if (event.key.keysym.sym == SDLK_LEFT && stan.Punkt_opadajacego_klocka.x + znajdz_najbardziej_na_lewo(stan.obecnie_opadajacy)*stan.rozmiar_klocka> 0) // sprawdzenie czy moze sie w lewo przesunac
						{
							if (czy_klocek_moze_w_lewo(stan))
								stan.Punkt_opadajacego_klocka.x -= delta*Plansza_szerokosc * 240;


							if (stan.Punkt_opadajacego_klocka.x + znajdz_najbardziej_na_lewo(stan.obecnie_opadajacy)*stan.rozmiar_klocka< 0)// implementacja gdyby wyszlo za ramke 
								stan.Punkt_opadajacego_klocka.x = abs(znajdz_najbardziej_na_lewo(stan.obecnie_opadajacy)*stan.rozmiar_klocka);
						}
						else if (event.key.keysym.sym == SDLK_p )
						{
							if (stan.pause == true && stan.koniec == false)
								stan.pause = false;
							else
								stan.pause = true;
						}
						

					else if (event.key.keysym.sym == SDLK_UP) // implementacja obrotu 
							obrot(&stan);
					else if(event.key.keysym.sym == SDLK_DOWN) // zrzut klocka na dol
						umiesc_na_dole(&stan);
					else if (event.key.keysym.sym == SDLK_r) restetuj(&stan, worldTime);
					break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_LEFT)
					{
						stan.Punkt_opadajacego_klocka.x = int(stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka )*stan.rozmiar_klocka;
						
					}
					
					if (event.key.keysym.sym == SDLK_RIGHT) // tylko wspolzedne o wielokrotnosci rozmiaru klocka
					{
						stan.Punkt_opadajacego_klocka.x = int(stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka + 0.99)*stan.rozmiar_klocka;

						if (!czy_klocek_moze_w_prawo(stan)) // sprawdzenie czy na prawo nie ma klocków
							stan.Punkt_opadajacego_klocka.x = int(stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka)*(stan.rozmiar_klocka) ;

						if (stan.Punkt_opadajacego_klocka.x / stan.rozmiar_klocka >
							Plansza_szerokosc - znajdz_najbardziej_na_prawo(stan.obecnie_opadajacy) )
						{ // implementacja gdyby wyszlo za ramke 
							stan.Punkt_opadajacego_klocka.x = (Plansza_szerokosc - znajdz_najbardziej_na_prawo(stan.obecnie_opadajacy) )*stan.rozmiar_klocka;
						}
					}
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_PRESSED:
				
					break;
				};
			};
		frames++;
		};

	
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
