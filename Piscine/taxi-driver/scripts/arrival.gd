extends Area2D

@onready var s1: Area2D = $"../s1"
@onready var s2: Area2D = $"../s2"
@onready var s3: Area2D = $"../s3"
@onready var s4: Area2D = $"../s4"
@onready var s5: Area2D = $"../s5"
@onready var s6: Area2D = $"../s6"
@onready var s7: Area2D = $"../s7"
@onready var s8: Area2D = $"../s8"

func _ready() -> void:
	connect("body_entered", Callable(self, "_on_body_entered"))
	randomize() # ensures randomness each game run
	
func _on_body_entered(body: Node) -> void:
	if not self.is_visible_in_tree():
		return

	if body.level_up():
		visible = false
		var pax = [s1,s2,s3,s4,s5,s6,s7,s8]
		var random_pax = pax[randi_range(0, pax.size() - 1)]
		random_pax.visible = true
