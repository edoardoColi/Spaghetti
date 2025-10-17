extends Area2D

@onready var casabl: Area2D = $"../casaBL"
@onready var casatl: Area2D = $"../casaTL"
@onready var casabr: Area2D = $"../casaBR"
@onready var casatr: Area2D = $"../casaTR"

func _ready() -> void:
	connect("body_entered", Callable(self, "_on_body_entered"))
	randomize() # ensures randomness each game run
	
func _on_body_entered(body: Node) -> void:
	if not self.is_visible_in_tree():
		return
	if body.load_pax():
		visible = false
		var casas = [casabl, casatl, casabr, casatr]
		var random_casa = casas[randi_range(0, casas.size() - 1)]
		random_casa.visible = true
