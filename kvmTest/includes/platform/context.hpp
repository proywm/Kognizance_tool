#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <functional>

//using namespace std;

typedef enum AccessType {LOAD, STORE, LOAD_AND_STORE, UNKNOWN} AccessType;
typedef enum FloatType {ELEM_TYPE_FLOAT16, ELEM_TYPE_SINGLE, ELEM_TYPE_DOUBLE, ELEM_TYPE_LONGDOUBLE, ELEM_TYPE_LONGBCD, ELEM_TYPE_UNKNOWN} FloatType;
typedef enum FunctionType {SAME_FN, DIFF_FN, UNKNOWN_FN} FunctionType;
typedef enum AddressSpaceType {KERNEL_HEAP, KERNEL_TASK_STACK, KERNEL_IRQ_STACK, KERNEL_NMI_STACK, KERNEL_EXCEPTION_STACK, USER_SPACE, UNKNOWN_ADDRESS_SPACE} AddressSpaceType;

class ContextFrame {
public:
	ContextFrame(){};
	~ContextFrame(){};
	ContextFrame(const ContextFrame &obj)
	{ 
		binary_addr = obj.binary_addr;
		method_name = obj.method_name;
		source_file = obj.source_file;
		src_lineno = obj.src_lineno;
	}
	bool operator==(const ContextFrame & obj) const
  	{
		return ((binary_addr == obj.binary_addr) && (src_lineno == obj.src_lineno));
	}

	long long binary_addr = 0;
	std::string method_name;
	std::string source_file;
	int src_lineno = 0;	
};

/*----------- Metric from witch --------------------------*/

//--------------------------- feels like unnecessary--------------
typedef enum {
    METRIC_VAL_REAL,
    METRIC_VAL_INT,
} metric_val_enum_t;

typedef struct {
    std::string client_name;
    std::string name;
    uint32_t threshold = 0;
    metric_val_enum_t val_type = METRIC_VAL_INT;
} metric_info_t;

//----------------------------------------------------------------*/



typedef struct {
    // double r = 0;
    int64_t r = 0;
    int64_t i = 0;

} metric_val_t;

inline metric_val_t operator+(const metric_val_t &v1, const metric_val_t &v2){
    metric_val_t sum;
    sum.r = v1.r + v2.r;
    sum.i = v1.i + v2.i;
    return sum;
}
inline metric_val_t &operator+=(metric_val_t &v1, const metric_val_t &v2){
    v1.r += v2.r;
    v1.i += v2.i;
    return v1;
}
inline bool operator<(const metric_val_t &v1, const metric_val_t &v2){
	return v1.i<v2.i;
}

class ContextMetrics {
public:
	bool increment(const metric_val_t &val)
	{ 
		metricVal += val;
		return true;
	}
//	metric_val_t *getMetricVal(int metric_idx);
	metric_val_t metricVal;
private:

//    std::map<int, metric_val_t*> _val_map;
//      metric_val_t metricVal;
//    friend xml::XMLObj * xml::createXMLObj<ContextMetrics>(ContextMetrics *instance);
//    friend class Context;
};
/*----------------------- end of Metric ---------------------------*/

class Context {
//private:
//	ContextFrame frame;
public:
	int id;
	//tool can have multiple metrics. It's tool's responsibility to handle metrics properly.
	// key denotes the type of Metrics. Tool defines the keys
	std::map<int, ContextMetrics> metrics;
	ContextFrame frame;
	
	bool operator==(const Context *obj) const
        {
                return (frame == obj->frame);
        }
	// Custom Hash Functor that will compute the hash on the
	// passed string objects length
	struct ContextHasher
	{
	  size_t
	  operator()(const Context * obj) const
	  {
	    return std::hash<int>()(obj->frame.src_lineno);
	  }
	};

	// Custom comparator that compares the string objects by length
	struct ContextComparator
	{
	  bool
	  operator()(const Context* obj1, const Context* obj2) const
	  {
	      return obj1->frame == obj2->frame;
	  }
	};

	std::unordered_set<Context*, ContextHasher, ContextComparator> children;
	Context *_parent = nullptr;
	Context(){}
	Context(long long addr, std::string method, std::string file, int lineNo)
	{
		frame.binary_addr = addr;
		frame.method_name = method;
		frame.source_file = file;
		frame.src_lineno = lineNo;
	}
	Context(ContextFrame *frameObj)
	{
		frame.binary_addr = frameObj->binary_addr;
                frame.method_name = frameObj->method_name;
                frame.source_file = frameObj->source_file;
                frame.src_lineno = frameObj->src_lineno;
	}
	~Context(){};
	std::string getContextFrame_methodName() {return frame.method_name;}

};

class Event {
public:
	Event(){};
	~Event(){
		if(valueAtEvent != nullptr)
			free(valueAtEvent);

		while (!eventContext_agent.empty())
  		{
			ContextFrame* cf = eventContext_agent.back();
			eventContext_agent.pop_back();
			delete cf;
		}

		while (!eventContext_host.empty())
                {
                        ContextFrame* cf = eventContext_host.back();
                        eventContext_host.pop_back();
                        delete cf;
                }
		
	};

	int eventId;
	int eventType;
	
	int deviceFileId;	/* source device */

	std::vector<ContextFrame*> eventContext_host;
	std::vector<ContextFrame*> eventContext_agent;
	int pId;
	int tId;

	/* For host environment */
	int h_pId;
	int h_tId;
	
	uint64_t ip_addr;
	unsigned long long data_addr;
	int cpu;

	/* For host environment */
	uint64_t h_ip_addr;		// ip within host environment
	unsigned long long h_data_addr;	//data address within host environment
	uint8_t *valueAtEvent = nullptr;
	uint64_t inst;
	int h_cpu;
	
	unsigned long long event_time;

	int event_type;	// load or store

	int data_src;// which memory it accessed
	int latency;
	
	int opcode = -1;	//instr Agent specific

	AccessType accessType = UNKNOWN;
	uint32_t accessLength = 0;
	FloatType floatType;

	AddressSpaceType addressSpaceType = UNKNOWN_ADDRESS_SPACE;
	
	bool fixSkid = false;
	bool isFixed = false;
	uint64_t fixedIP = 0;

	uint64_t sample_index = 0;

	/* Signal Info */
//	int signum;
//	siginfo_t *info;
//	void *context;

};


#endif
