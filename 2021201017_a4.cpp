#include<iostream>
#include<string.h>
#include<bits/stdc++.h>
#include<unistd.h>
#include<fcntl.h>
using namespace std;


#define BLOCKS 128000	//128000*4 kb
#define B_SIZE 4092	//4 kb -4 bytes for int(next block)
#define DATA_BLOCKS 127300
#define INODE_BLOCKS 600



struct super_block{
	int inode_count;
	int block_count;
	int data_blocks;
	int size_blocks;
	
};


struct inode{
	int size_file;
	char name_file[32];
	int pointer[12];
	int first_block;
};

struct disk_block{
	int next_block;
	char data[B_SIZE];
};



void create_disk();
void mount_disk(string);
void sync_fs(string);
void printt();
int get_free_inode();
int get_free_block();

void options(string);

void create_file(string);
void open_file(string);
void read_file(int);
int write_file(int);
void append_file(string);
void close_file(int);
void delete_file(string);
void list_files();
void list_open_files();



struct super_block sbl;
struct inode *ino;
struct disk_block* db;
vector<string> list_f;
vector< pair<string,int> > list_of;
vector<string>desc;



void create_disk(){

	int i;
	
	sbl.block_count=BLOCKS;
	sbl.data_blocks=DATA_BLOCKS;
	
	sbl.size_blocks=sizeof(struct disk_block);
	sbl.inode_count= sbl.size_blocks/(sizeof(struct inode));	
	sbl.inode_count= sbl.inode_count*INODE_BLOCKS;
	
	//init inodes
	ino = new inode[sbl.inode_count];
	for(i=0;i<sbl.inode_count; i++){
		ino[i].size_file=-1;
		strcpy(ino[i].name_file,"empty file");
		ino[i].first_block=-1;
	}
	
	
	//init disk blocks
	db=new disk_block[sbl.data_blocks];
	for(i=0;i<sbl.data_blocks;i++){
		db[i].next_block=-1;
	}

}

void sync_fs(string st){
	int i;
	FILE *fp;
	char* name=&st[0];
	
	fp=fopen(name,"w+");
	
	char buff[B_SIZE];
	
	//init sbl
	fwrite(&sbl,sizeof(struct super_block),1,fp);	


	//ino = new inode[sbl.inode_count];
	for(i=0;i<sbl.inode_count; i++){
		fwrite(&ino[i],sizeof(struct inode),1,fp);
	}
	
	
	//init disk blocks
	//db=new disk_block[sbl.data_blocks];
	for(i=0;i<sbl.data_blocks;i++){
		fwrite(&db[i],sizeof(struct disk_block), 1, fp);
	}
	
	fclose(fp);

}



void mount_disk(string st){
	int i;
	FILE *fp;
	char*name=&st[0];
	
	fp=fopen(name,"r");
	
	char buff[B_SIZE];
	
	//init sbl
	fread(&sbl,sizeof(struct super_block),1,fp);	
	
	ino = new inode[sbl.inode_count];
	db=new disk_block[sbl.data_blocks];
	
		
	for(i=0;i<sbl.inode_count; i++){
		fread(&ino[i],sizeof(struct inode),1,fp);
	}
	
	//init disk blocks
	for(i=0;i<sbl.data_blocks;i++){
		fread(&db[i],sizeof(struct disk_block), 1, fp);
	}
	
	
	
	fclose(fp);
}


void printt(){

	int i;
	cout<<"SBL size"<<sizeof(struct super_block)<<endl;
	cout<<"Size of blocks: "<<sbl.size_blocks<<endl;
	cout<<"indoe size:"<<(sizeof(struct inode))<<endl;
	cout<<"Count in "<<INODE_BLOCKS<<"  blocks: "<<sbl.inode_count<<endl;
	cout<<"Count of data blocks: "<<sbl.data_blocks<<endl;
	
	
	for(i=0;i<sbl.inode_count; i++){
		cout<<"INODE "<<i<<" fs "<<ino[i].size_file<<" first "<<ino[i].first_block<<" "<<ino[i].name_file<<endl;
	}
	
	
	for(i=0;i<sbl.data_blocks;i++){
		cout<<"BLOCK "<<i<<" "<<db[i].next_block<<" "<<db[i].data<<endl;
	}
	
	for(i=0;i<list_f.size();i++){
		cout<<list_f[i]<<" ";
	}
	
	cout<<endl;
	

}




int get_free_inode(){
	int i;
	
	for(i=0;i<sbl.inode_count; i++){
		if(ino[i].first_block == -1 ){
			return i;
		}
	}
	
	return -1;
}

int get_free_block(){
	int i;
	
	for(i=0;i<sbl.block_count; i++){
		if(db[i].next_block==-1){
			return i;
		}
	}
	
	return -1;
}



void create_file(string fname){
	int i,fin, fda;
	
	//get free inode and free data block
	fin=get_free_inode();
	fda=get_free_block();
	//cout<<"FIN "<<fin<<" FDA "<<fda<<endl;
	
	list_f.push_back(fname);
	
	if(fin==-1){
		cout<<endl<<"No more free inodes.";
		return;
	}
	
	if(fda==-1){
		cout<<endl<<"No more free data blocks.";
		return;		
	}
	
	ino[fin].first_block = fda;
	strcpy(ino[fin].name_file,fname.c_str());
	
	db[fin].next_block=-10; //means in use

}

void open_file(string name){
	int ch,i,ind=-1;
	
	cout<<"Enter mode: (R=0   W=1   A=2) ";
	cin>>ch;
	
	if(ch>2||ch<0){
		cout<<"Nota valid mode"<<endl;
		return;
	}
	
	for(i=0;i<sbl.inode_count; i++){
		if( !strcmp(ino[i].name_file,name.c_str()) ){
		
		cout<<ino[i].name_file<<" ";
			ind=i;		
		}
	
	}
	
	if(ind==-1){
		cout<<"Couldnt open"<<endl;
		return;
	}
	
	
	list_of.push_back({name,ch});
	
	cout<<endl<<"FILE DESCRIPTOR: "<<list_of.size()-1+3<<" MODE "<<ch<<endl;
	
}

void read_file(int fnum){

	int loc,i,fb;
	char buf[B_SIZE];
	
	if(fnum-3 >= list_of.size()){
		cout<<"Please check fd once more."<<endl;
		return;	
	}
	
	string fname=list_of[fnum-3].first;

	
	if(list_of[fnum-3].second!=0){
		cout<<"Not in read mode."<<endl;
		return;
	}
	
	
	for(i=0;i<sbl.inode_count; i++){
		if( !strcmp(ino[i].name_file, fname.c_str()) ){
			loc=i;
			break;
		}
	
	}
	
	fb=ino[loc].first_block;
	
	while(1){
		strcpy(buf,db[fb].data);
		cout<<buf;
		fb=db[fb].next_block;
		
		if(fb==-10 || fb==-1)
			break;
	}
	
	cout<<endl<<"DONE READ"<<endl;

}

int write_file(int fnum){
	
	int loc,i,fb,j,len=0,coun=0,nb,cb;
	char buf[B_SIZE];
	string data_s[B_SIZE];
	string s="";
	char x;
	
	
	if(fnum-3 >= list_of.size()){
		cout<<"Please check fd once more."<<endl;
		return -1;	
	}
	
	string fname=list_of[fnum-3].first;

	
	if(list_of[fnum-3].second!=1){
		cout<<"Not in write mode."<<endl;
		return -1;
	}
	
	
	for(i=0;i<sbl.inode_count; i++){
		if( !strcmp(ino[i].name_file, fname.c_str()) ){
			loc=i;
			break;
		}
	
	}
	
	fb=ino[loc].first_block;
	
	cout<<"Enter file content. Press ; to finish."<<endl;

	while(1){
		x=getchar();
		
		if(x==';'){
			data_s[i]=s;
			i++;
			break;
		}
		
		s+=x;
		coun++;
		
		if(coun>=B_SIZE){
			coun=0;
			data_s[i]=s;
			s="";
			i++;
		
		}			
	}
	
	len=i;	
	nb=fb;
	cb=fb;
	
	//clear existing content
	
	while(1){
		
		if(nb==-10){
			db[cb].next_block=-1;
			break;
		}
		
		
		cb=nb;
		nb=db[cb].next_block;
		db[cb].next_block=-1;
		
		
	}
	db[fb].next_block=-10;
	
	
	//write new content
	vector<int>roll_back;
	cb=fb;
	nb=fb;
	
	for(i=0;i<len;i++){
		
		//cout<<"NB "<<nb<<" cB "<<cb<<" STR "<<data_s[i]<<endl;
		cb=nb;
		strcpy(db[cb].data,data_s[i].c_str());
		
		
		nb=get_free_block();
		
		
		roll_back.push_back(nb);
		
		
		if(nb==-1){
			roll_back.pop_back();
			cout<<"NO MORE SPACE ON DISK. File too big. File content removed.";
			//ROLLBACK.
			db[fb].next_block=-10;
			strcpy(db[fb].data,"");
			for(j=0;j<roll_back.size();j++){
			//cout<<" RB "<<roll_back[j];
				db[ roll_back[j] ].next_block=-1;
			}
			
			return -1;
		}
		
		db[cb].next_block=nb;
		db[nb].next_block=-10;
	}
	

	db[cb].next_block=-10;
	db[nb].next_block=-1;
	
	//cout<<"FB CB NB"<<fb<<" "<<cb<<" "<<nb<<endl;
	
	db[cb].next_block=-10;
	
	return 1;

}

void append_file(int fd){
	string fname;
	int mode,fb,cb,nb,i,j,loc,len=0,coun=0;
	char buf[B_SIZE];
	string data_s[B_SIZE];
	string s="";
	char x;
	
	if(fd-3 >= list_of.size()){
		cout<<"Please check fd once more."<<endl;
		return;	
	}
	
	
	fname=list_of[fd-3].first;
	mode=list_of[fd-3].second;
	
	if(mode!=2){
		cout<<"Not in append mode.."<<endl;
		return;
	}
	
	for(i=0;i<sbl.inode_count; i++){
		if( !strcmp(ino[i].name_file, fname.c_str()) ){
			loc=i;
			break;
		}
	}	
	
	fb=ino[loc].first_block;
	cb=fb;
	nb=fb;
	
	while(1){
		
		if(nb==-10){
			break;
		}
		cb=nb;
		nb=db[cb].next_block;
	}
	
	//append
	//cout<<"CB:"<<cb<<" NB:"<<nb<<endl; 

	//write
	cout<<"Enter file content to append. Press ; to finish."<<endl;

	while(1){
		x=getchar();
		
		if(x==';'){
			data_s[i]=s;
			i++;
			break;
		}
		
		s+=x;
		coun++;
		
		if(coun>=B_SIZE){
			coun=0;
			data_s[i]=s;
			s="";
			i++;
		
		}			
	}
	
	len=i;
	fb=cb;
	nb=get_free_block();
	
	db[fb].next_block=nb;
	db[nb].next_block=-10;
	
	vector<int>roll_back;
	
	for(i=0;i<len;i++){
		
		//cout<<"NB "<<nb<<" cB "<<cb<<" STR "<<data_s[i]<<endl;
		
		cb=nb;
		strcpy(db[cb].data,data_s[i].c_str());
		
		
		if(i==0){
			roll_back.push_back(cb);}
		
		nb=get_free_block();
		roll_back.push_back(nb);
		
		
		if(nb==-1){
			roll_back.pop_back();
			cout<<"NO MORE SPACE ON DISK. Rolling back changes";
			//ROLLBACK.
			db[fb].next_block=-10;
			
			for(j=0;j<roll_back.size();j++){
				db[ roll_back[j] ].next_block=-1;
			}
			
			return;
		}
		
		db[cb].next_block=nb;
		db[nb].next_block=-10;
	}
	

	db[cb].next_block=-10;
	db[nb].next_block=-1;
	
	//cout<<"CB NB"<<cb<<" "<<nb<<endl;
	
	db[cb].next_block=-10;
	

}

void close_file(int fd){
//remove from list_of
//remove from mode

	list_of.erase(list_of.begin()+fd-3);
	
	cout<<"FILE descriptors have changed. Please recheck before further operations.";

}

void delete_file(string name){

	//remove from list_f and list_of
	int fl,i,j,fb,cb,nb,loc=0;
	fl=0;
	vector<int>ind_l;
	
	for(i=0;i<list_f.size();i++){
		if(name==list_f[i]){
			fl=1;
			list_f.erase(list_f.begin()+i);
			break;
		}
	}	
	
	if(!fl){
		cout<<"Please check filename again.."<<endl;
		return;
	}
	
	for(i=0;i<list_of.size();i++){
		if(name==list_of[i].first){
			ind_l.push_back(i);
		}
	}
	
	for(i=ind_l.size()-1;i>=0;i--){
		list_of.erase(list_of.begin()+ind_l[i]);
	}
	
	
	
	
	//go to that inode and make first block -1
	for(i=0;i<sbl.inode_count; i++){
		if( !strcmp(ino[i].name_file, name.c_str()) ){
			loc=i;
			break;
		}
	}
	
	fb=ino[i].first_block;
	strcpy(ino[i].name_file,"empty file");
	ino[i].first_block=-1;
	
	//delete all data blocks next
	cb=fb;
	nb=fb;
	 
	while(1){
		nb=db[cb].next_block;
		if(nb==-10){
			db[cb].next_block=-1;
			break;
		}
		db[cb].next_block=-1;
		cb=nb;
		
	}
	
	cout<<"Done delete "<<name;

}

void list_files(){

	for(int i=0;i<list_f.size(); i++){
		cout<<list_f[i]<<" ";
	}
	cout<<endl;

}
void list_open_files(){

	for(int i=0;i<list_of.size(); i++){
		cout<<"File name: "<< list_of[i].first<<" desc: "<<i+3<<" mode: "<<list_of[i].second<<endl;
	}
	cout<<endl;

}


void populate_list_files(){

	string s;
	int i;
	for(i=0;i<sbl.inode_count; i++){
		string s(ino[i].name_file);
		if(s=="empty file"){
			break;
		} 
		
		list_f.push_back(s);
	}
}


void options(string d_name){
	int choice=0,fd,ret;
	string name;
	
	list_f.clear();
	populate_list_files();
	
	while(choice>=0){
	
	cout<<endl<<"1:Create file\t 2:Open file \t 3:Read file \t 4:Write file \t 5:Append \t 6:Close \t 7:Delete \t 8:List of files \t 9:List of open files \t 10.Unmount"<<endl;

	cin>>choice;
	
	switch(choice){
	
		case 1:
			cin>>name;
			create_file(name);
			sync_fs(d_name);
			break;
		case 2:
			cin>>name;
			open_file(name);
			break;
		case 3:
			cout<<"Enter fd"<<endl;
			cin>>fd;
			read_file(fd);
			break;
		case 4:
			cout<<"Enter fd"<<endl;
			cin>>fd;
			ret=write_file(fd);
			
			//if(ret==1){
				sync_fs(d_name);
			//}
			break;
			
		case 5:
			cout<<"Enter fd"<<endl;
			cin>>fd;
			append_file(fd);
			sync_fs(d_name);
			break;
		case 6:
			cout<<"Enter fd"<<endl;
			cin>>fd;
			close_file(fd);
			break;
		case 7:
			cin>>name;
			delete_file(name);
			sync_fs(d_name);
			break;
		case 8:
			list_files();
			break;
		case 9:
			list_open_files();
			break;
		case 10:
			//unmount();
			//write to fs
			list_of.clear();
			choice=-1;
			break;
		
		default:
			cout<<"WRONG CHOICE..";
			choice=-1;
			break;
	
	}
	
	}

}

int main(){
	int choice;
	string name;
	
	while(choice>=0){
	cout<<"1: Create a disk\t2: Mount Disk\t3:Exit "<<endl;
		cin>>choice;
		
		switch(choice){
			case 1:
				cin>>name;
				
				if(access(&name[0],F_OK)!=-1){
					cout<<"Disk already exists."<<endl;
					break;
				}
				
				
				create_disk();
				sync_fs(name);
				break;
				
			case 2:
				cin>>name;
				
				if(access(&name[0],F_OK)==-1){
					cout<<"Disk doesnt exist..."<<endl;
					break;
				}
				
				mount_disk(name);
				options(name);
				//printt();
				
				break;
			
			case 3:
				choice=-1;
				break;
					
			
		}
		
		
		cout<<endl;
	
	}
	
	cout<<"Exitted application."<<endl;

}

/*

Following the sudden, violent deaths of the last two fully legitimate members of the Ptolemaic family in Egypt, the people of Alexandria in 80 invited Ptolemy XII to assume the throne. Although he was known as a son of Ptolemy IX Soter II, his mother was a mistress of Soter, not a wife. In 103 he was sent by his grandmother, Cleopatra III, queen of Egypt, in the company of his brother and Ptolemy XI Alexander II, his predecessor, to Cos, an Aegean island near Asia Minor, for safekeeping. Captured in 88 by Mithradates VI Eupator, ruler of Pontus, a kingdom in Asia Minor that was then at war with Rome, young Ptolemy appeared in 80 in Syria, from where, according to Cicero, he arrived in Egypt, while his brother became king of Cyprus.

*/
