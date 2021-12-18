#include<stdio.h>
#include<stdlib.h>

/*
	this is a program to  run a memory manager in a assigned malloc chunk 
	It's purpose is to help me understand the glibc's malloc function's work
	However , because of my poor skill and understand in C programming 
	I can't figure out How to rebuild this struct .
	So it's only a simple rebuild of it. some data struct is designed by myself , 
	inspired by  linux .
	
	And i'm not native English speaker ,Plz forgive my poor English.

*/


/*
	bugs: 2018.12.19
	found bugs in big chunk and mediem, size amount out of int , plz try long long to fix it.
	or use 2 int  that pre_size to fix it. (or may be use pre_size that no longer to sort.)
*/




/*
	Copy right :suppose not for homework and comercial for security reason.
	

*/

#define LINKSIZE 1024
#define DATASIZE 1024*1024*1024 //1G
//i should define a size of struct size maybe.



const int chunk_size[6]={0,16,32,64,128,256};

//This struct is from CTF-WiKI's heap page. I change some of it to simplify this program.
typedef struct malloc_free_chunk {

  int     prev_size;  /* Size of previous chunk (if free).  */
  int     size;       /* Size in bytes, including overhead. */
  struct malloc_free_chunk* fd;         /* double links -- used only if free. */
  struct malloc_free_chunk* bk;			/*if used we can use this to point the data*/
  /*if it's a big chunk(size more than 256) we use 2 char position to make a INT(16bit) to save it.
  	
  */
  
  
  /* Only used for large blocks: pointer to next larger size.  */
  /*struct malloc_chunk* fd_nextsize;*/ /* double links -- used only if free. */
  /*struct malloc_chunk* bk_nextsize;*/
	  
}Chunk;
/*
	if a chunk is free it's last bit will be 0 else it will be 1.
	and because the reassign of chunk ,all chunk's size must be specific number
	like 0 16 32 64 128 256.

*/




/*
	fastbin is design for the small chunk that size small than 256byte.
	when it was free this chunk's data point will be saved in a specific array 
	all similer chunk freed will be linked together and if the user want a same size's chunk.
	it will be assigned from the fastbin first.
	


*/





static char* Data ;
static Chunk* fastbin[6];
void* Malloc(int data_size);
Chunk* Link_assigned(int data_size);
Chunk * mediem_chunk ;
char* Top_chunk;


void* Malloc(int data_size)
{
	Chunk* chunk=Link_assigned(data_size);
	int * s =(int*)chunk;
	s-=2;
	chunk=s;
	if(chunk->size > 255)
	{//not finished;
		
		if(chunk->size >64*1024)
		{//large chunk just give it.
			printf("assigned successful size of chunk is:%d\n",chunk->size-1);
		}
		else
		{// mediem chunk check freed first.
			printf("assigned successful size of chunk is:%d\n",chunk->size-1);
		}
	}else
	{
		printf("assigned successful size of chunk is:%d\n",chunk->size-1);
	
	}
	s+=2;
	chunk=s;
	return chunk;
}

char * GetHead()
{
	
	return Top_chunk;//Top_chunk must be add the data_size after assigned. 
}


Chunk* Link_assigned(int data_size)
{
	int* Head=NULL;
	if(data_size>256)
	{//not finished
		
		/*																		 64*1024bit	
			   There are 3 kind of chunk size : fastbin(<256) ; mediem chunk(256~128k)  ; 	 large chunk(>128k).
			   However ,The size of the chunk used in daily programming are often small chunks
			   So using a fastbin can accelerate it . when use mediem chunk  we use common free principle ,
			   all freed chunk is linked in one linklist, no need to know there location in memory.
			   check each chunk is big enough to assigned a chunk to the new request.
			   when free ,To prevent the memory broken , we need to check the pre chunk(physical location) is freed or not if freed and not a fastbin chunk
			   then put them together and rewrite the size and other information of the sum chunk. 
			   when assigned :
			   if it's , then give it. 
			   if not  ,then seek the bk point chunk is suitable enough . 
			   if still not , then ask the malloc to give a new chunk that enough for the request from the Top of the chunk,
			   remember to check is the Top is over the request memory from the system. 
			   when assigned large chunk, i suppose there always no enough freed chunk to give it.So why not just assigned from the Top.
			   LOL, and still we need to split the memory into piece that can be mod 2.  i think 1024's N time is OK.(N is at least 64)
			   When free the large chunk , just put it into the mediem chunk's free Linklist.
			   it will atuomaticly reuse it.
			   If you need the large chunk frequently , just create a linklist like mediem chunk's way, it will became faster.
			   
			   
			   new chunk request ----->  judge the size  ------>  not fastbin  ->  check the freed chunk        check the Top is over the max memory -> not   -> give the chunk 
												|											|					 ^										|
												| is the fastbin						enough   -> not  --------|										Yes -> return ERRO_no_eough_chunk
												|											|	
												|										give the chunk
												|										  
												|									 
											assigned with fastbin
		
		*/
		
		
		
		if(data_size >= 64*1024)  //when assigned with 65536 it will over the limit 
		{//large chunk just give it.
			int chunk_size_large;
			if(data_size%1024!=0)
			{
			   chunk_size_large=(data_size/1024+1)*1024;// I forget the '/' will up or not . PLZ check it !!!!!!!
			}
			else
			{
			   chunk_size_large=data_size;
			}
			  
			Head=(int *)Top_chunk;
			Head+=1;
			*Head=chunk_size_large+1;//put the flag;
			Top_chunk+=chunk_size_large; 
			Head+=1; 
			return Head;
			
		}
		else
		{// mediem chunk check freed first.
			//check freed chunk first
			//adjust to a 512 base memory chunk
			Chunk * current_chunk=mediem_chunk;
			int freed_chunk_size=0;
			do
			{
				if(current_chunk->bk!=NULL)
				{
					//check if it's proper to give
					current_chunk=current_chunk->bk;	
				}
				else
				{
					//No enough mediem free chunk
					Head=-1;
					break;	
				}
				
				
			}while(current_chunk->size <= data_size);
		
			//No freed chunk left.
			int chunk_size_mediem;
			if(data_size%512!=0)
			{
				 chunk_size_mediem=(data_size/512+1)*512;// I forget the '/' will up or not . PLZ check it !!!!!!!
			}
			else
			{
				 chunk_size_mediem=data_size;
			}
			
			
			if(Head==-1)
			{	
				//No enough mediem free chunk ,and give it a new chunk
			//	int chunk_size_mediem=(data_size/512+1)*512;
				Head=(int *)Top_chunk;
				Head+=1;
				*Head=chunk_size_mediem+1;
				Top_chunk+=chunk_size_mediem;
				Head+=1;
				return Head;	
			}
			else
			{
				//have enough mediem free chunk
				if(Head==NULL)
				{
					freed_chunk_size=current_chunk->size;
					Head=(int *)current_chunk;
					char * left_chunk= Head;
					int left_chunk_size;
					left_chunk+=chunk_size_mediem;
					
			
					// first cut the chunk with size needed.  fin
					// seconed record the cut chunk and left chunk(if have)  fin    current_chunk is cut and the left_chunk is left.
					// third add the left chunk into the fastbin or mediem chunk ( depend on the size  of left chunk)
					// finally return the cut chunk.
					left_chunk_size=freed_chunk_size - chunk_size_mediem;
					if(left_chunk_size<=256)
					{
						if(left_chunk_size==0)
						{//no left : just delete
					
								if( current_chunk->bk != NULL)
								{	
									//not the last
									current_chunk->fd->bk=current_chunk->bk;
									current_chunk->bk->fd=current_chunk->fd;
								}
								else
								{
									//last chunk node;
									current_chunk->fd->bk=NULL;
									current_chunk->fd =NULL;
								
								}
							
							
						}
						else
						{  // left  is fastbin : add to fastbin list
						
								if( current_chunk->bk != NULL)
								{	
									//not the last
									current_chunk->fd->bk=current_chunk->bk;
									current_chunk->bk->fd=current_chunk->fd;
								}
								else
								{
									//last chunk node;
									current_chunk->fd->bk=NULL;
									current_chunk->fd =NULL;
								
								}
							
							
							 int * tmp1 = (int *)left_chunk;   // maybe by the special adjust way ,it will got 512 for the small chunk
							 tmp1+=1;
							 *tmp1=left_chunk_size;
							 *tmp1+=1;
							 tmp1+=1;
							 Free(tmp1);
							 
							
						}
					}
					else
					{
					
								if( current_chunk->bk != NULL)
								{	
									//not the last
									current_chunk->fd->bk=current_chunk->bk;
									current_chunk->bk->fd=current_chunk->fd;
								}
								else
								{
									//last chunk node;
									current_chunk->fd->bk=NULL;
									current_chunk->fd =NULL;
								
								}
				
						int *tmp1 = (int *)left_chunk;
						tmp1+=1;
						*tmp1=left_chunk_size;
						*tmp1+=1;
						tmp1+=1;
						Free(tmp1);
						
					}
					
					
				}
				Head+=1;
				*Head=chunk_size_mediem;
				*Head+=1;
				Head+=1;
				
				return Head;  //maybe problem
			}
			
			
		}
		
		return NULL;
	}
	else
	{
	
		/*
		check the reqire is matched for the freed chunk first
		the order is fastbin ,freed big chunk ,then go to the Top_chunk to assigned a suitable chunk.	
		
		Not finished yet.
		
		*/
		
		if(0)//seem no uesd in this if
		{
			
		}else
		{  //no matched chunk and the size is smaller than 256
			Head=GetHead();
							
				if(data_size<=64)
				{					
					if(data_size<32)
					{ // 16 32
						if(data_size<=16)   // chunk_size 0 is "0"
						{//16
							if(fastbin[1]->bk!=NULL)
							{//if fastbin have
								Head=(int *)fastbin[1]->bk;
								if(fastbin[1]->bk->bk!=NULL)
								{//more than one
									
									fastbin[1]->bk=fastbin[1]->bk->bk;
									fastbin[1]->bk->fd=fastbin[1];
								}else
								{//only one
									fastbin[1]->bk=NULL;
									
								}
								//this pointer is point to data   , if point to data it's wrong to use that struct , PLZ change not to match it
								//However i need to -1(int)to match it
							}
							else
							{
								Top_chunk+=sizeof(int)*2;
								Top_chunk+=chunk_size[1]; 
							}
							Head++;
							*Head=chunk_size[1];
							*Head+=1;
							Head+=1;
						  				  //Top chunk should not add  when used freed chunk;
							return Head;  // this head return Data position
						}
						else
						{//32
							if(fastbin[2]->bk!=NULL)
							{//if fastbin have
								Head=(int *)fastbin[2]->bk;
								if(fastbin[2]->bk->bk!=NULL)
								{//more than one
									
									fastbin[2]->bk=fastbin[2]->bk->bk;
									fastbin[2]->bk->fd=fastbin[2];
								}else
								{//only one
									fastbin[2]->bk=NULL;
									
								}
								//this pointer is point to data
								//However i need to -1(int)to match it
							}
							else
							{
								Top_chunk+=sizeof(int)*2;
								Top_chunk+=chunk_size[2];	
							}
							Head++;
							*Head=chunk_size[2];
							*Head+=1;
							Head+=1;
							
							return Head;
						}
							
					}
					else
					{ //64
							if(fastbin[3]->bk!=NULL)
								{//if fastbin have
								Head=(int *)fastbin[3]->bk;
								if(fastbin[3]->bk->bk!=NULL)
								{//more than one
									
									fastbin[3]->bk=fastbin[3]->bk->bk;
									fastbin[3]->bk->fd=fastbin[3];
								}else
								{//only one
									fastbin[3]->bk=NULL;
									
								}
								//this pointer is point to data
								//However i need to -1(int)to match it
						
							}
							else
							{
								Top_chunk+=sizeof(int)*2;
								Top_chunk+=chunk_size[3];	
							}
						Head++;
						*Head=chunk_size[3];
						*Head+=1;
						Head+=1;
						return Head;
					}
				}else
					{
						if(data_size<=128)
						{//128
							if(fastbin[4]->bk!=NULL)
							{//if fastbin have
								Head=(int *)fastbin[4]->bk;
								if(fastbin[4]->bk->bk!=NULL)
								{//more than one
									
									fastbin[4]->bk=fastbin[4]->bk->bk;
									fastbin[4]->bk->fd=fastbin[4];
								}else
								{//only one
									fastbin[4]->bk=NULL;
									
								}
								//this pointer is point to data
								//However i need to -1(int)to match it
								
							}
							else
							{
								Top_chunk+=sizeof(int)*2;
								Top_chunk+=chunk_size[4];
							}
							Head++;
							*Head=chunk_size[4];
							*Head+=1;
							Head+=1;
							return Head;
						
						}else
						{//256
							if(fastbin[5]->bk!=NULL)
							{//if fastbin have
								Head=(int *)fastbin[5]->bk;
								if(fastbin[5]->bk->bk!=NULL)
								{//more than one
									
									fastbin[5]->bk=fastbin[5]->bk->bk;
									fastbin[5]->bk->fd=fastbin[5];
								}else
								{//only one
									fastbin[5]->bk=NULL;
									
								}
								//this pointer is point to data
								//However i need to -1(int)to match it
							
							}
							else
							{
								Top_chunk+=sizeof(int)*2;
								Top_chunk+=chunk_size[5];
							}
							Head++;
							*Head=chunk_size[5];
							*Head+=1;
							Head+=1;
							return Head;
						}
					}
			
		}		
				
	}
	//return chunk;
}


/*
	if a chunk mediem is free , i suppose to check if it's physical memory  near is freed or not.
	if freed , it's better to combine them together. And refreshed the Linklist of mediem. 
	if a chunk large is free , i suppose to put it into the mediem linklist .


*/

//void check_pre_free_chunk(Chunk * chunk , )
//{
	
	
//}



int Free(Chunk * chunk) //it will give a point in Data  area
{
	int data_size=0;
	int* s =(int *)chunk;
	s-=2;  // s is the orignal start of chunk struct
	chunk=(Chunk *)s;
	if(chunk!=NULL&&chunk->size&1==1)
	{
		data_size=(chunk->size-1);
		if(data_size>256+1)
		{
			if(data_size>=64*1024)
			{//large chunk
				Chunk * current_chunk=mediem_chunk;
				int mark=0;
				do
				{	//until it's bigger or same but final
					if(current_chunk->bk!=NULL)
					{
						//check if it's proper to give
						
						current_chunk=current_chunk->bk;
					}
					else
					{
						//No enough mediem free chunk
						if(current_chunk->size > chunk->size -1)
						{
							mark=0;
						}
						else
						{
							mark=-1;
						}
						break;	
					}
					
				}while(current_chunk->size <= chunk->size-1);
			
				switch(mark)
				{
					case -1:
						{
							current_chunk->bk=chunk;
							chunk->bk=NULL;
							chunk->fd=current_chunk;
						}break;
					case 0:
						{
							chunk->fd =current_chunk;
							chunk->bk =current_chunk->bk;
							current_chunk->bk=chunk;
							chunk->bk->fd=chunk;
						}break;
				
					default :
						{
							printf("UNKNOW MARK");
							return 1;
						}break;
				}
				
				
				
				if(mediem_chunk->bk!=NULL)
				{
					
					if(mediem_chunk->bk!=NULL)
					{//add to the lastest chunk
						mediem_chunk->bk->fd=chunk;
						chunk->bk=mediem_chunk->bk;
						mediem_chunk->bk=chunk;
						chunk->fd=mediem_chunk;
					}
					else
					{
						mediem_chunk->bk=chunk;
						chunk->fd=mediem_chunk;	
					}	
				}
				chunk->size-=1;
				return 0;	
			}
			else
			{//mediem chunk
				Chunk * current_chunk=mediem_chunk;
				int mark=0;
				do
				{	//until it's bigger or same but final
					if(current_chunk->bk!=NULL)
					{
						//check if it's proper to give
						
						current_chunk=current_chunk->bk;
					}
					else
					{
						//No enough mediem free chunk
						if(current_chunk->size > chunk->size -1)
						{  //current is bigger than the chunk.
							// add it before current.
							mark=0;
						}
						else
						{  // last not OK
							// if the last is not bigger than freed chunk
							//you should add the chunk to the last
							mark=-1;
						}
						break;	
					}
					
				}while(current_chunk->size <= chunk->size-1);
				
				switch(mark)
				{
					case -1:
						{
							current_chunk->bk=chunk;
							chunk->bk=NULL;
							chunk->fd=current_chunk;
						}break;
					case 0:
						{	
							chunk->bk=current_chunk;
							current_chunk->fd->bk=chunk;								  //   current_chunk   
							chunk->fd =current_chunk->fd;             //   chunk-1         chunk    chunk +1 
							current_chunk->fd=chunk;
						}break;
				
					default :
						{
							printf("UNKNOW MARK");
							return 1;
						}break;
				}
		/*		
				if(mediem_chunk->bk!=NULL)
				{
					
					if(mediem_chunk->bk!=NULL)
					{//add to the lastest chunk
						mediem_chunk->bk->fd=chunk;
						chunk->bk=mediem_chunk->bk;
						mediem_chunk->bk=chunk;
						chunk->fd=mediem_chunk;
					}
					else
					{
						mediem_chunk->bk=chunk;
						chunk->fd=mediem_chunk;	
					}	
				}
	*/	
				chunk->size-=1;
				return 0;	
			}
		}
		else
		{
			if(data_size<=64)
			{		
				if(data_size<=32)
					{ // 16 32
						if(data_size<=16)   // chunk_size 0 is "0"
							{//16
									if(fastbin[1]->bk!=NULL)
									{
										fastbin[1]->bk->fd=chunk; //  here got a logic bug. for the insert linklist.  just give the orignal and +1 to fix it.
										chunk->bk=fastbin[1]->bk;
											
									}
									fastbin[1]->bk=chunk;
									chunk->size=0;
							}
						else
							{//32
									if(fastbin[2]->bk!=NULL)
									{
										fastbin[2]->bk->fd=chunk;
										chunk->bk=fastbin[2]->bk;
											
									}
									fastbin[2]->bk=chunk;	
									chunk->size=0;
							}
							
					}
					else
					{ //64
								if(fastbin[3]->bk!=NULL)
								{
									fastbin[3]->bk->fd=chunk;					
									chunk->bk=fastbin[3]->bk;
								}
								fastbin[3]->bk=chunk;	
								chunk->size=0;	
					}
			}
			else
			{
						if(data_size<=128)
						{//128
							if(fastbin[4]->bk!=NULL)
							{
								fastbin[4]->bk->fd=chunk;
								chunk->bk=fastbin[4]->bk;								
							}
							fastbin[4]->bk=chunk;	
							chunk->size=0;
						}
						else
						{//256
							if(fastbin[5]->bk!=NULL)
							{
								fastbin[5]->bk->fd=chunk;	
								chunk->bk=fastbin[5]->bk;					
							}
							fastbin[5]->bk=chunk;	
							chunk->size=0;
						}
			}
		}
		
				
		return 0;	
	}
	else
	{
		printf("This is a freed chunk Plz check\n");
		return 1;	
	}
	
}
int main()
{
	//init process
	char * malloc_chunk_Data = (char*)malloc(sizeof(char)*DATASIZE);
	
	Data=malloc_chunk_Data;
	
	for(int i=0;i<6;i++)
	{
		fastbin[i]=NULL;
	}
	Top_chunk=Data;
	memset(Data,0,DATASIZE);
	Chunk * fast16=(Chunk*)malloc(sizeof(Chunk));
	Chunk * fast32=(Chunk*)malloc(sizeof(Chunk));
	Chunk * fast64=(Chunk*)malloc(sizeof(Chunk));
	Chunk * fast128=(Chunk*)malloc(sizeof(Chunk));
	Chunk * fast256=(Chunk*)malloc(sizeof(Chunk));
	fast16->bk=NULL;
	fast32->bk=NULL;
	fast64->bk=NULL;
	fast128->bk=NULL;
	fast256->bk=NULL;
	fastbin[1]=fast16;
	fastbin[2]=fast32;
	fastbin[3]=fast64;
	fastbin[4]=fast128;
	fastbin[5]=fast256;
	mediem_chunk=(Chunk *)malloc(sizeof(Chunk));
	mediem_chunk->fd=NULL;
	mediem_chunk->bk=NULL;
	mediem_chunk->size=0;
	
	//main start from here
	

		
	//free memory from OS
	free(malloc_chunk_Data);
	malloc_chunk_Data=NULL;
	return 0;
}


