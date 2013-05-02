bool disassembler(char *filename, Hash *mn){
	char *dltfile = strdup(filename);
	dltfile[strlen(filename)-3] = 'd';
	dltfile[strlen(filename)-2] = 'l';
	dltfile[strlen(filename)-1] = 't';

	FILE *fin = fopen(filename, "r"),
		 *fout = fopen(dltfile, "w");

	if(fin == NULL)
		return true;

	char *name = (char *)calloc(7, sizeof(char)),
		 *cbase = (char *)calloc(7, sizeof(char)),
		 *ctotal = (char *)calloc(7, sizeof(char)),
		 *buf = (char *)calloc(Tokenizer_Max_Length, sizeof(char)),
		 *buff = (char *)calloc(7, sizeof(char));
	unsigned int start, base, total, i, j, k;
	
	fscanf(fin, "H%6c%6c%6c\n", name, cbase, ctotal);
	total = hex2int(ctotal);
	start = base = hex2int(cbase);
	memset(ctotal, '\0', 7);
	memset(cbase, '\0', 7);

	fprintf(fout, "%04X\t%s\tSTART\t%04X\n", base, name, base);


	while(fscanf(fin, "T%6c%2c%s\n", cbase, ctotal, buf) == 3){

		base = hex2int(cbase);

		for(i=0;i<hex2int(ctotal);i++){
			memset(buff, '\0', 7);

			fprintf(fout, "%04X\t", base + i);

			buff[0] = buf[i*2];
			buff[1] = buf[i*2+1];
			OPMNNode *found = OP_Search(mn, buff);

			if(found != NULL){
				fprintf(fout, "%s\t",found->mnemonic);

				i++;
				buff[2] = buf[i*2];
				buff[3] = buf[i*2+1];

				bool _x_ = false;
				if(buff[2] > '8'){
					_x_ = true;
					switch(buff[2]){
						case '9':
							fprintf(fout, "1");
							break;
						case 'A':
							fprintf(fout, "2");
							break;
						case 'B':
							fprintf(fout, "3");
							break;
						case 'C':
							fprintf(fout, "4");
							break;
						case 'D':
							fprintf(fout, "5");
							break;
						case 'E':
							fprintf(fout, "6");
							break;
						case 'F':
							fprintf(fout, "7");
							break;
					}
					fprintf(fout, "%c", buff[3]);
				}else{
					fprintf(fout, "%s", buff+2);
				}

				i++;
				buff[4] = buf[i*2];
				buff[5] = buf[i*2+1];
				fprintf(fout, "%s", buff+4);
				if(_x_)
					fprintf(fout, ",X");
				fprintf(fout, "\t%s", buff);
			}else{
				for(j=i+1;j<hex2int(ctotal);j++){
					memset(buff, '\0', 7);
					buff[0] = buf[j*2];
					buff[1] = buf[j*2+1];
					OPMNNode *found = OP_Search(mn, buff);
					if(found != NULL)
						break;
				}
				if(!((j-i)%3)){
					for(k=i;k<j;k+=3){
						memset(buff, '\0', 7);
						buff[0] = buf[k*2];
						buff[1] = buf[k*2+1];
						buff[2] = buf[k*2+2];
						buff[3] = buf[k*2+3];
						buff[4] = buf[k*2+4];
						buff[5] = buf[k*2+5];
						fprintf(fout, "%s\t\t%s", buff, buff);
					}
					i=j-1;
				}else{
					char *d = (char *)calloc(j-i, sizeof(char));
					for(k=i;k<j;k++){
						d[(k-i)*2] = buf[k*2];
						d[(k-i)*2+1] = buf[k*2+1];
					}
					fprintf(fout, "%s\t\t%s", d, d);
					free(d);
					i=j-1;
				}
			}
			fprintf(fout, "\n");
		}
	}
	{
		char *found = (char *)calloc(7, sizeof(char));
		fscanf(fin, "E%6c", found);
		fprintf(fout, "\tEND\t%4X\n", hex2int(found));
		free(found);
	}
	printf("\toutput file : [%s]\n", dltfile);

	free(name);
	free(cbase);
	free(ctotal);
	free(buf);
	fclose(fin);
	fclose(fout);
	return false;
}
