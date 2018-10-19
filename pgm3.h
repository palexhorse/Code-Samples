#DEFINE max 1000;
class DAGNode {
	public:
		int num;
		int delay;
		int sourcecheck=1;
		DAGNode *successorshead;
		DAGNode *successorstail;
		DAGNode *next;

	DAGNode (int n, int del){
		num=n;
		delay=del;
		successorshead=successorstail=NULL;
		next=NULL;
	}
	
	void add_successor(int x, int y){
		DAGNode *walk;
		DAGNode *temp;
		walk=successortail;
		if (walk==NULL){
			temp = new DAGNode(x, y);
			successorshead=temp;
			successorstail=successorshead;
		}
		else{
			temp = new DAGNode(x, y);
			walk->next=temp;
			successortail=walk->next;
			successortail->next = NULL;
		}
	}
};

class DRS{
	public:
		int tour[100];
		int tour_crit[100];
		int depth;
		int critical;
		int depth_crit;
		int sum;
	DRS(){
		depth=0;
		sum=0;
		critical=0;
	}
};

class DAG{
	public:
	int arctable[max][max];
	int crewcount;
	int sink;
	int sources[100];
	int sourcecount;
	DRS schedule;
	DAGNode DAGArray[1000];
	DAGNode *walk;
	DAGNode *temp;
	
	DAG(){
		int x, y, arc;
		char command;
		sourcecount=0;
		cin >> crewcount;
		while (cin.good()) {
			cin >> command;
			if (command == 'c'){
				cin >> nodecount;
			}
			else if (command == 'a') {
				// Read in arc data
				cin >> x >> y >> arc;
				arctable[x][y] = arc;
				DAGArray[y].sourcecheck=0;
				DAGArray[x].add_successor(y, arc);
				}
		}
		y=0;
		// Find source nodes and sink node
		for(x=0; x<nodecount+1; x++){
			if(DAGArray[x].sourcecheck==1){
				source[y]=x;
				y++;
				sourcecount++;
			}
			else if(DAGArray[x]->successorshead==NULL;){
				sink=x;
			}
		}
	// function to move traverse paths to sink	
	void lookahead(int a, int x){
		DAGNode *anchor = DAGArray[a];
		DAGNode *traverse = anchor.successorlisthead;
		while (traverse!=NULL){
			schedule.depth++;
			schedule.sum+=traverse.delay;
			schedule.tour[x]=traverse.num;
			int z=traverse.num;
			lookahead(z, x++);
			if (schedule.sum > schedule.critical){
				schedule.critical = schedule.sum;
				schedule.depth_crit = schedule.depth;
				for(q=0; q<schedule.depth; q++){
					schedule.tour_crit[q]=schedule.tour[q];
				}
			}
			traverse=traverse->next;
		}
	
	void critical(){
		int i;
		for (y=0; y<sourcecount; y++){
			i=sources[y];
			walk = DAGArray[i].successorshead;
			while (walk != NULL){
				lookahead(i, x);
				if (schedule.sum > schedule.critical){
					schedule.critical = schedule.sum;
					for(q=0; q<schedule.depth; q++){
						tour_crit[q]=tour[q];
					}
				}
				walk=walk->next;
			}
		}
	cout<<"The length of the critical path is "<<schedule.critical<<endl;
	cout<<"The nodes on the citical path are : {"<<schedule.tour_crit[0];
	for (q=1 q<schedule.depth_crit; q++){
		cout<<", "<< schedule.tour_crit[q];
	}
	cout<<"}"<<endl;
	}