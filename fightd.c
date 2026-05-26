/* 
 * modifed: igorpauk 2017-18
 *
 * Fight Server
 * "The Legend: The Legacy of the dragons" project
 * Arseny Vakhrushev (Sen), it-territory, 2006-2007
 *
 */

#define REV "Fight Server, $Revision: 1.16 $"


#include "common.h"
#include "srv.h"
/*#include "md5.h"*/

char           *fs_host = NULL;
int            fs_ctrlPort = 0;
int            fs_clPort = 0;
debugLevel_t   fs_debugLevel = DL_WARN;
char           *fs_debugLog = NULL;
char           *fs_feedbackUrl = NULL;
char           *checkStr = NULL;
//MD5    			digestMd5;

void parseCmdLine(int argc, char *argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "h:p:c:l:d:f:")) != -1) {
		switch ((unsigned char)opt) {
			case 'h':
				fs_host = strdup(optarg);
				break;
			case 'p':
				fs_ctrlPort = atoi(optarg);
				break;
			case 'c':
				fs_clPort = atoi(optarg);
				break;
			case 'l':
				fs_debugLevel = atoi(optarg);
				break;
			case 'd':
				fs_debugLog = strdup(optarg);
				break;
			case 'f':
				fs_feedbackUrl = strdup(optarg);
				break;
			case 'n':
				checkStr = strdup(optarg);
				break;
		}
	}
}

//const size_t BufferSize = 144*7*1024;
//char* buffer = new char[BufferSize];

int main(int argc, char *argv[]) {
	parseCmdLine(argc,argv);
	DEBUG_INIT(fs_debugLevel,fs_debugLog,REV);
	if (!fs_ctrlPort || !fs_clPort || (fs_ctrlPort == fs_clPort)) {
		MSG("Usage: %s [-h <host>] -p <ctrl_port> -c <client_port> [-l <debug level>] [-d <debug log>] [-f <http feedback URL>]",argv[0]);
	} else {
		
		/*while (*fs_host){
			fs_host->read(buffer, BufferSize);
			std::size_t numBytesRead = size_t(input->gcount());
			digestMd5.add(buffer, numBytesRead);
		}
		
		if(digestMd5.getHash() != checkStr){
			return 0;
		}*/
		
		if ((fs_srvInit(fs_host,fs_ctrlPort,fs_clPort) == OK) && (fs_feedbackInit(fs_feedbackUrl) == OK)) {
			fs_srvRun();
		}
		fs_srvDone();
		fs_feedbackDone();
	}
	DEBUG_DONE();
	free(fs_debugLog);
	free(fs_feedbackUrl);
	return 0;
}
