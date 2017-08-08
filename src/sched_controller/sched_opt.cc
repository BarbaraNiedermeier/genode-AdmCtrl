/*
 * \brief  
 * \author Barbara Niedermeier
 * \date   2017/07/25
 */

#include <base/printf.h>

/* for optimize function */
#include <util/xml_node.h>
#include <util/xml_generator.h>
#include <typeinfo>
/* ******************************** */

#include "sched_controller/sched_opt.h"

#include <vector>


namespace Sched_controller {

	// Definition of the optimization goal via xml file
	void Sched_opt::set_goal(Genode::Ram_dataspace_capability xml_ds_cap)
	{
		Genode::Rm_session* rm = Genode::env()->rm_session();
		const char* xml = rm->attach(xml_ds_cap);		
		PDBG("Optimizer - Start parsing XML file.");
		Genode::Xml_node root(xml);

		const auto fn = [this] (const Genode::Xml_node& node)
		{
			int max_len = 32;
			std::vector<char> fair_goal(max_len);
			std::vector<char> util_goal(max_len);
			
			// store xml node value
			node.sub_node("fairness").sub_node("apply").value(fair_goal.data(), fair_goal.size());
			node.sub_node("utilization").sub_node("apply").value(util_goal.data(), util_goal.size());
			
			
			
			// analyze xml node value and set _opt_goal
			if (std::stoi(fair_goal.data())) 
			{
				// optimization goal is set to fairness
				_opt_goal = FAIRNESS;
			}
			else if (std::stoi(util_goal.data()))
			{
				// optimization goal is set to utilization
				_opt_goal = UTILIZATION;
			}
			else
			{
				// no optimization goal is set
				_opt_goal = NONE;
			}
			
			
		};
		root.for_each_sub_node("goal", fn);
		rm->detach(xml);
		
		PDBG("Optimizer - Finish parsing XML file.");
	}
	
	
	void Sched_opt::add_task(int core, Rq_task::Rq_task task)
	{
		
		PDBG("Optimizer - Add task to task list.");
		// Es kommt immer nur ein neuer Task hinzu. Dieser Muss identifiziert werden und in das Task-Array eingefügt werden.
		// es sollte pro core ein task-array geben, bei dem die Tasks gespeichert sind, die bei diesem Task an/abgeschaltet werden sollen
		
		// pro core gibt es eine RunQueue
		
		// convert newly arriving task to optimization-task model
		Optimization_task _task;
		
		//_task.name = task.name; // das geht erst, wenn Masterarebit von Steffan fertig ist
		_task.deadline = task.deadline;
		_task.inter_arrival = task.inter_arrival;
		_task.task_ptr = nullptr;
		
		
		_tasks.push_back(_task);
		
		// update RunQueue
		
		// so fügt man einen Task am Ende der run_queue hinzu 
		//_rqs[core].enq(task);
	}
	
	void Sched_opt::start_optimizing()
	{
		
		switch( _opt_goal )
		{
			case NONE:
				PDBG("The optimization goal 'none' has been chosen.");
				break;
			case FAIRNESS:
				PDBG("The optimization goal 'fairness' has been chosen.");
				break;
			case UTILIZATION:
				PDBG("The optimization goal 'utilization' has been chosen.");
				break;	
			default:
				PDBG("No optimization goal has been chosen.");
				
		}
	}
	
	void Sched_opt::_deactivate_task()
	{
		// To-Do: Beeinflusse den rq_buffer so, dass min-task nicht ausgeführt wird, dafür ein anderer
		// rq_buffer ~ _rqs[core]
		
		
		
		// Zuerst den rq_buffer aktualisieren!
		// Wie mache ich das? Siehe Steffan
		
		// bestimme den Task, der nicht wieder in die RunQueue eingefügt werden soll.
		//char task_name[24];
		
		// je nach Optimieruns-Ziel, wird entweder der Task mit min value aus RunQueue gelöscht, oder der task mit _min_utilization
		
		switch( _opt_goal )
		{
			case NONE:
				// don't delete any task fron the run queue
				break;
			case FAIRNESS:
				// lösche den Tast, mit _min_utilization aus RunQue
				//task_name = _tasks[_min_task].name;
				break;
			case UTILIZATION:
				// lösche den Tast, mit _min_utilization aus RunQue
				break;	
			default:
				PDBG("No optimization goal has been set.");
				
		}
		
		
		
		// wenn man speziellen Task löschen will, muss ich alle Tasks raus löschen und in richtiger reihenfolge wieder einfügen?
		
		//so löscht man den ersten Task in der run_queue, der Pointer zum Task wird in task_ptr gespeichert
		// _rqs[core].deq(task_ptr)
		
		// Pointer des gelöschten Tasks in Task-Array speichern
	}
	
	Sched_opt::Sched_opt(Mon_manager::Connection *_mon_man, Genode::Dataspace_capability mon_ds_cap)
	{
		_mon_manager = _mon_man;
		_mon_ds_cap = mon_ds_cap;
		
		// as default, no optimization is done
		_opt_goal = NONE;
		
		// initialize task list
		//_tasks = new Optimization_task[0];
	}
	
	
	Sched_opt::~Sched_opt()
	{
	}


}
