#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct RGB {

    unsigned char R;
    unsigned char G;
    unsigned char B;
}pixel;

union sv{
    unsigned int x;
    unsigned char c[3];
};

union xor{
    unsigned int y;
    unsigned char c[3];
};
//unions used for the encrypting algorithm

unsigned int xorshift32(unsigned int seed){
	//function that returns random unsigned int
    unsigned int r=seed;

        r = r^r<<13;
        r = r^r>>17;
        r = r^r<<5;

    return r;
}

unsigned int *permute(int width, int height, unsigned int *random){

    unsigned int *pixels = (unsigned int*)malloc((width*height)*sizeof(unsigned int));
    int i, aux, j;

    for(i=0; i < width*height; i++)
        pixels[i] = i;

    for(i=width*height-1; i>=1; i--){
        j = random[width*height-i]%(i+1);
        aux = pixels[i];
        pixels[i] = pixels[j];
        pixels[j] = aux;
    }

    return pixels;
}

pixel* linearization (char* bmp_filename){

    FILE *fs = fopen(bmp_filename,"rb");

    if(fs == NULL)
        printf("eroare(linializare)");

    rewind(fs);

    unsigned int width, height;
    fseek(fs, 18, SEEK_SET);
    fread(&width, sizeof(unsigned int), 1, fs);
    fread(&height,sizeof(unsigned int), 1, fs);

    int padding, i, j;

    if(width%4 != 0)
        padding=4-(3*width)%4;
    else
        padding=0;

    fseek(fs, 54, SEEK_SET);
    pixel* pxArray = (pixel*)malloc(height*width*sizeof(pixel));
    
    for(i=height-1; i>=0; i--){
        for(j=0; j<width; j++){
            fread(&pxArray[i*width+j].B,1,1,fs);
            fread(&pxArray[i*width+j].G,1,1,fs);
            fread(&pxArray[i*width+j].R,1,1,fs);
        }
        fseek(fs,padding,SEEK_CUR);
    }
    return pxArray;
}

void export_linear(char* bmp_filename,char* bmp_out, pixel* linear){

    FILE *f,*g;
    f=fopen(bmp_filename, "rb");
    g=fopen(bmp_out, "wb+");

    if(f==NULL || g==NULL){
        printf("error(export_linear)");
        exit(0);
    }

    unsigned int width, height;
    fseek(f,18,SEEK_SET);
    fread(&width,sizeof(unsigned int),1,f);
    fread(&height,sizeof(unsigned int),1,f);

    int padding, i, j;
    if(width%4 != 0)
        padding = 4-(3*width)%4;
    else
        padding = 0;

    unsigned char z;
    fseek(f,0,SEEK_SET);

    for(int i=0; i<54; i++){
        fread(&z, 1, 1, f);
        fwrite(&z, 1, 1, g);
    }
    unsigned char k=0;

    for(i=height-1; i>=0;i--){
        for(j=0; j<width; j++){
            fwrite(&linear[i*width+j].B, 1, 1, g);
            fwrite(&linear[i*width+j].G, 1, 1, g);
            fwrite(&linear[i*width+j].R, 1, 1, g);
        }
        fwrite(&k, sizeof(unsigned char), padding, g);
    }

}

void encrypt(char* bmp_in,char *txt_key, char* bmp_out){

    FILE *fin,*key;

        fin = fopen(bmp_in, "rb");
        key=fopen(txt_key, "r");

        if(fin==NULL || key==NULL){
            printf("error(encrypt)");
            exit(0);
        }

    unsigned int width, height, KEY, SV;

        fseek(fin, 18, SEEK_SET);
        fread(&width, sizeof(unsigned int), 1, fin);
        fread(&height, sizeof(unsigned int), 1, fin);
        rewind(fin);

        fscanf(key, "%u", &KEY);
        fscanf(key, "%u", &SV);
        fclose(key);


        union sv form;
        form.x = SV;
        union xor rnd;

        unsigned int *R = (unsigned int *)malloc((2*height*width)*sizeof(unsigned int));
        R[0] = KEY;

        for(int i=1; i<=2*width*height-1; i++)
            R[i] = xorshift32(R[i-1]);

        unsigned int *perm = permute(width, height, R);
            pixel *L = linearization(bmp_in);
            pixel *P = (pixel*)malloc(width*height*sizeof(pixel));

            for(int x=0;x<height*width;x++){
                P[perm[x]].B=L[x].B;
                P[perm[x]].G=L[x].G;
                P[perm[x]].R=L[x].R;
            }

        pixel* C=(pixel*)malloc(width*height*sizeof(pixel));

        for (int k=0; k<height*width; k++){

                rnd.y=R[height*width+k];

                if(k==0){

                    C[k].B=form.c[0]^P[k].B^rnd.c[0];
                    C[k].G=form.c[1]^P[k].G^rnd.c[1];
                    C[k].R=form.c[2]^P[k].R^rnd.c[2];
                }
                else{
                    C[k].B=C[k-1].B^P[k].B^rnd.c[0];
                    C[k].G=C[k-1].G^P[k].G^rnd.c[1];
                    C[k].R=C[k-1].R^P[k].R^rnd.c[2];
                }

        }

                export_linear(bmp_in, bmp_out, C);
                fclose(fin);

}
void decrypt(char *bmp_in, char* bmp_out, char *txt_key){

        FILE *key,*fin;
        key=fopen(txt_key,"r");
        fin=fopen(bmp_in,"rb");
        if(key==NULL || fin==NULL)
        {
            printf("error(decrypt)");
            exit(0);
        }

        unsigned int width, height, cheie, SV;
                rewind(fin);
                fseek(fin,18,SEEK_SET);
                fread(&width,sizeof(unsigned int),1,fin);
                fread(&height,sizeof(unsigned int),1,fin);
                fscanf(key,"%u",&cheie);
                fscanf(key,"%u",&SV);
                fclose(key);

                union sv form;
                form.x=SV;
                union xor rnd;

                unsigned int *R=(unsigned int *)malloc((2*height*width)*sizeof(unsigned int));
                R[0]=cheie;

                for(int i=1;i<=2*width*height-1;i++)
                    R[i]=xorshift32(R[i-1]);

            unsigned int *perm=permute(width,height,R);

            int *inv_perm=(int*)malloc(width*height*sizeof(int));

            pixel *L=linearization(bmp_in);

            for(int i=0; i<width*height; i++){

                inv_perm[perm[i]]=i;
            }

            pixel *C=(pixel*)malloc(height*width*sizeof(pixel));

            for (int k=0;k<height*width;k++){

                    rnd.y=R[height*width+k];

                   if(k==0){

                       C[k].B=form.c[0]^L[k].B^rnd.c[0];
                       C[k].G=form.c[1]^L[k].G^rnd.c[1];
                       C[k].R=form.c[2]^L[k].R^rnd.c[2];

                    }
                    else{

                        C[k].B=L[k-1].B^L[k].B^rnd.c[0];
                        C[k].G=L[k-1].G^L[k].G^rnd.c[1];
                        C[k].R=L[k-1].R^L[k].R^rnd.c[2];
                    }

            }
            pixel *D=(pixel*)malloc(width*height*sizeof(pixel));


            for(int x=0;x<height*width;x++){

                    D[inv_perm[x]].B=C[x].B;
                    D[inv_perm[x]].G=C[x].G;
                    D[inv_perm[x]].R=C[x].R;
            }


            export_linear(bmp_in, bmp_out, D);

}
void test_chi(char *nume_imagine){

    FILE *f;
    f=fopen(nume_imagine,"rb");
    rewind(f);

    if(f==NULL){
        printf("eroare(test_chi)");
        exit(0);}

    unsigned int latime,inaltime,i;
    fseek(f,18,SEEK_SET);
    fread(&latime,sizeof(unsigned int),1,f);
    fread(&inaltime,sizeof(unsigned int),1,f);
    fclose(f);

    pixel*px=linearization(nume_imagine);

    unsigned int*fvR,*fvG,*fvB;
    fvR=(unsigned int*)calloc(256,sizeof(unsigned int));
    fvG=(unsigned int*)calloc(256,sizeof(unsigned int));
    fvB=(unsigned int*)calloc(256,sizeof(unsigned int));

    for(i=0;i<inaltime*latime; i++)
    {
        fvB[px[i].B]++;
        fvG[px[i].G]++;
        fvR[px[i].R]++;

    }

    double fb=(inaltime*latime)/256;

    double *sum=(double*)calloc(3,sizeof(double));

        for(i=0;i<256;i++){
            sum[0]+=(fvR[i]-fb)*(fvR[i]-fb)/fb;
            sum[1]+=(fvG[i]-fb)*(fvG[i]-fb)/fb;
            sum[2]+=(fvB[i]-fb)*(fvB[i]-fb)/fb;
        }

    printf("Rezultatul testului chi patrat : \nR:%.2f \nG:%.2f \nB:%.2f",sum[0],sum[1],sum[2]);
    free(fvR);
    free(fvG);
    free(fvB);
    }
int main()
{
    char* bmp_in = "LAND2.BMP";
    char* bmp_out = "ENCR_TEST.bmp";
    char* key = "key.txt";

    //encrypt(bmp_in, key, bmp_out);
    decrypt(bmp_out, "decrypted.bmp", key);

    	

    return 0;
}
