#ifndef	CHIP_BUS_SCHEDULE_H
#define	CHIP_BUS_SCHEDULE_H

#ifdef __cplusplus
extern "C"
{
#endif

    struct notify_msg
    {
	int     msg_type;
	void    *msg_private;
	void    *msg_param;
    };

    typedef void    (*notify_register_node_priv)(isil_register_node_t *, void *, isil_notify_msg *);
    typedef int     (*match_regsiter_node_priv_id)(isil_register_node_t *, void *, unsigned long );

    struct isil_register_node_operation{
	void	(*init_complete)(isil_register_node_t *);
	void	(*wait_complete)(isil_register_node_t *);
	void	(*complete_done)(isil_register_node_t *);
	void	(*init_wait_suspend_complete)(isil_register_node_t *);
	void	(*wait_suspend_complete)(isil_register_node_t *);
	void	(*suspend_complete_done)(isil_register_node_t *);
	void	(*init_wait_resume_complete)(isil_register_node_t *);
	void	(*wait_resume_complete)(isil_register_node_t *);
	void	(*resume_complete_done)(isil_register_node_t *);
	void    (*init_all_complete)(isil_register_node_t *);
	void    (*wait_all_complete)(isil_register_node_t *);
	void    (*all_complete_done)(isil_register_node_t *);
    };

#define	to_get_register_node(node)		container_of(node, isil_register_node_t, list)
    struct isil_register_node{
	struct list_head	list;
	struct completion	wait_stop;
	struct completion	wait_suspend;
	struct completion	wait_resume;

	void	*register_node_priv;
	notify_register_node_priv	notify_priv;
	match_regsiter_node_priv_id	match_id;

	struct isil_register_node_operation	*op;
    };

    struct register_table_node_operation{
	void    (*register_node_into_table)(isil_register_table_t *, isil_register_node_t *);
	void    (*unregister_node_from_table)(isil_register_table_t *, isil_register_node_t *);
	void    (*find_register_node_in_table)(isil_register_table_t *, isil_register_node_t **, unsigned long );
	void    (*get_node_from_table)(isil_register_table_t *, isil_register_node_t **,unsigned long);
	int     (*get_curr_register_node_number)(isil_register_table_t *);
    };

    struct register_table_node{
	spinlock_t          lock;
	struct list_head    register_header;
	atomic_t            curr_register_entry_number;

	struct register_table_node_operation	*op;
    };

    struct isil_root_bus{
	isil_register_table_t	root_register_table;
    };

    struct type_bus_operation{
	void	(*init)(type_bus_t *, int );

	void	(*register_chip_bus_into_type_bus_table)(type_bus_t *bus, isil_chip_bus_t *chip_bus);
	void	(*unregister_chip_bus_from_type_bus_table)(type_bus_t *bus, isil_chip_bus_t *chip_bus);
	void	(*find_chip_bus_in_type_bus_table)(type_bus_t *bus, isil_chip_bus_t **chip_bus, int chip_id);
	int     (*get_device_id)(type_bus_t *bus);
    };

#define	to_get_type_bus_with_register_node(node)	container_of(node, type_bus_t, register_node)
    struct type_bus{
	isil_register_node_t	register_node;
	isil_register_table_t	type_bus_root_register_table;
	int			bus_id;
	atomic_t    device_id;

	struct type_bus_operation	*op;
    };

    struct chip_bus_operation{
	void	(*init)(isil_chip_bus_t *, int , int);
    };

#define	to_get_chip_bus_with_chip_register_node(node)	container_of(node, isil_chip_bus_t, chip_register_node)
    struct isil_chip_bus{
	isil_register_node_t	chip_register_node;
	int	bus_id;
	int	chip_id;

	struct chip_bus_operation	*op;
    };

    struct robust_processing_control_operation{
	void    (*init)(robust_processing_control_t *robust);
	void    (*start_robust_processing)(robust_processing_control_t *robust);
	void    (*robust_processing_done)(robust_processing_control_t *robust);
	void    (*wait_robust_processing_done)(robust_processing_control_t *robust);
	int     (*is_in_robust_processing)(robust_processing_control_t *robust);
    };

    struct robust_processing_control{
	atomic_t    robust_processing_flag;
	struct completion   robust_processing_done;

	struct robust_processing_control_operation  *op;
    };

    extern void	get_root_bus(isil_root_bus_t **ptr_root_bus);
    extern void	get_isil_host_bus(type_bus_t **ptr_type_bus);
    extern void	get_isil_pci_bus(type_bus_t **ptr_type_bus);
    extern void get_isil_dpram_controller(dpram_control_t **ptr_isil_dpram_controller);
    extern void	gen_isil_bus_event(void);
    extern void	wait_isil_bus_event(void);

    extern void	init_isil_root_bus(void);
    extern void	remove_isil_root_bus(void);
    extern void	init_type_bus(type_bus_t *bus, int bus_id);
    extern void	remove_type_bus(type_bus_t *bus);
    extern void	init_chip_bus(isil_chip_bus_t *chip_bus, int bus_id, int chip_id);
    extern void	remove_chip_bus(isil_chip_bus_t *chip_bus, int bus_id);
    extern void	init_endpoint_tcb(ed_tcb_t *ed_tcb, int bus_id, int chip_id, int type_id, int group_chan_id, int logic_chan_id, void *priv, notify_register_node_priv notify_priv, match_regsiter_node_priv_id match_id, fsm_state_transfer_matrix_table_t *fsm_table);
    extern void	init_register_table_node(isil_register_table_t *table);
    extern void	init_register_node(isil_register_node_t *node, void *priv, notify_register_node_priv notify_priv, match_regsiter_node_priv_id match_id);
    extern void init_robust_processing_control(robust_processing_control_t *robust);

#ifdef __cplusplus
}
#endif

#endif	//CHIP_BUS_SCHEDULER_H

