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


unsigned int xorshift32(unsigned int seed){

    unsigned int r=seed;

        r = r^r<<13;
        r = r^r>>17;
        r = r^r<<5;

    return r;
}

unsigned int *permutare(int w,int h,unsigned int *random){

    unsigned int *perm = (unsigned int*) malloc( (w*h)*sizeof(unsigned int) );

    int k,aux;

    for(k=0; k<w*h; k++)
        perm[k]=k;

    int j;

    for(k=w*h-1; k>=1; k--){

        j=random[w*h-k]%(k+1);
        aux=perm[k];
        perm[k]=perm[j];
        perm[j]=aux;
    }
    return perm;
}
pixel* liniarizare(char* nume_imagine){

    FILE *f=fopen(nume_imagine,"rb");
    if(f==NULL)
    {
        printf("eroare(linializare)");
    }
    rewind(f);

    unsigned int latime,inaltime;
    fseek(f,18,SEEK_SET);
    fread(&latime,sizeof(unsigned int),1,f);
    fread(&inaltime,sizeof(unsigned int),1,f);

    int padd;
    if(latime%4 != 0)
        padd=4-(3*latime)%4;
    else
        padd=0;

    fseek(f,54,SEEK_SET);
    pixel* pxArray=(pixel*)malloc(inaltime*latime*sizeof(pixel));
    int i,j;
    for(i=inaltime-1; i>=0; i--){
        for(j=0; j<latime; j++){
            fread(&pxArray[i*latime+j].B,1,1,f);
            fread(&pxArray[i*latime+j].G,1,1,f);
            fread(&pxArray[i*latime+j].R,1,1,f);
        }
        fseek(f,padd,SEEK_CUR);
    }
    return pxArray;
}

void export_lin(char* nume_imagine_h,char* nume_img_out,pixel*v){

    FILE *f,*g;
    f=fopen(nume_imagine_h,"rb");
    g=fopen(nume_img_out,"wb+");

    if(f==NULL || g==NULL){
        printf("eroare(export_lin)");
    }

    unsigned int latime,inaltime;
    fseek(f,18,SEEK_SET);
    fread(&latime,sizeof(unsigned int),1,f);
    fread(&inaltime,sizeof(unsigned int),1,f);

    int padd;
    if(latime%4 != 0)
        padd=4-(3*latime)%4;
    else
        padd=0;

    unsigned char z;
    fseek(f,0,SEEK_SET);

    for(int x=0;x<54;x++){
        fread(&z,1,1,f);
        fwrite(&z,1,1,g);
    }
    int i,j;
    unsigned char k=0;
    for(i=inaltime-1; i>=0;i--){

        for(j=0;j<latime;j++){

            fwrite(&v[i*latime+j].B,1,1,g);
            fwrite(&v[i*latime+j].G,1,1,g);
            fwrite(&v[i*latime+j].R,1,1,g);
        }
        fwrite(&k,sizeof(unsigned char),padd,g);
    }

    }
void cripteaza(char* nume_fin,char *nume_cheie,char*nume_fout){

    FILE *fin,*key;

        fin=fopen(nume_fin,"rb");
        key=fopen(nume_cheie,"r");

        if(fin==NULL || key==NULL){
            printf("eroare(cripteaza)");
            exit(0);
        }

    unsigned int inaltime,latime,cheie,SV;

        fseek(fin,18,SEEK_SET);
        fread(&latime,sizeof(unsigned int),1,fin);
        fread(&inaltime,sizeof(unsigned int),1,fin);
        rewind(fin);

        fscanf(key,"%u",&cheie);
        fscanf(key,"%u",&SV);
        fclose(key);


        union sv form;
        form.x=SV;
        union xor rnd;

        unsigned int *R=(unsigned int *)malloc((2*inaltime*latime)*sizeof(unsigned int));
        R[0]=cheie;

        for(int i=1; i<=2*latime*inaltime-1; i++)
            R[i]=xorshift32(R[i-1]);

        unsigned int *perm=permutare(latime,inaltime,R);
            pixel *L=liniarizare(nume_fin);
            pixel *P=(pixel*)malloc(latime*inaltime*sizeof(pixel));

            for(int x=0;x<inaltime*latime;x++){
                P[perm[x]].B=L[x].B;
                P[perm[x]].G=L[x].G;
                P[perm[x]].R=L[x].R;
                        }


        pixel* C=(pixel*)malloc(latime*inaltime*sizeof(pixel));

        for (int k=0;k<inaltime*latime;k++){

                rnd.y=R[inaltime*latime+k];

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

                export_lin(nume_fin,nume_fout,C);
                fclose(fin);

}
void decripteaza(char *nume_fin,char* nume_img_enc,char*nume_fout,char *nume_cheie){

        FILE *key,*fin;
        key=fopen(nume_cheie,"r");
        fin=fopen(nume_fin,"rb");
        if(key==NULL || fin==NULL)
        {
            printf("eroare(decripteaza)");
            exit(0);
        }

        unsigned int inaltime,latime,cheie,SV;
                rewind(fin);
                fseek(fin,18,SEEK_SET);
                fread(&latime,sizeof(unsigned int),1,fin);
                fread(&inaltime,sizeof(unsigned int),1,fin);
                fscanf(key,"%u",&cheie);
                fscanf(key,"%u",&SV);
                fclose(key);

                union sv form;
                form.x=SV;
                union xor rnd;

                unsigned int *R=(unsigned int *)malloc((2*inaltime*latime)*sizeof(unsigned int));
                R[0]=cheie;

                for(int i=1;i<=2*latime*inaltime-1;i++)
                    R[i]=xorshift32(R[i-1]);

            unsigned int *perm=permutare(latime,inaltime,R);

            int *inv_perm=(int*)malloc(latime*inaltime*sizeof(int));

            pixel *L=liniarizare(nume_img_enc);

            for(int i=0; i<latime*inaltime; i++){

                inv_perm[perm[i]]=i;
            }

            pixel *C=(pixel*)malloc(inaltime*latime*sizeof(pixel));

            for (int k=0;k<inaltime*latime;k++){

                    rnd.y=R[inaltime*latime+k];

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
            pixel *D=(pixel*)malloc(latime*inaltime*sizeof(pixel));


            for(int x=0;x<inaltime*latime;x++){

                    D[inv_perm[x]].B=C[x].B;
                    D[inv_perm[x]].G=C[x].G;
                    D[inv_perm[x]].R=C[x].R;
            }


            export_lin(nume_fin,nume_fout,D);


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

    pixel*px=liniarizare(nume_imagine);

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
    
    //cripteaza("LAND2.BMP","key.txt","encrypted.bmp");

    decripteaza("encrypted.bmp", "encrypted.bmp", "decrypted.bmp", "key.txt");
    	

    return 0;
}
