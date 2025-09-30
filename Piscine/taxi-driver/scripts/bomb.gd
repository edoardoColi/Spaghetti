extends Area2D

func _ready() -> void:
	connect("body_entered", Callable(self, "_on_body_entered"))

func _on_body_entered(body: Node) -> void:
	body.explode_for(2.0) # freeze car for 2 seconds
	queue_free() # optional: bomb disappears
