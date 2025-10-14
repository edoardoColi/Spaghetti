extends Control

@onready var meter_label: Label = $Score/meters
@onready var pause_menu: NinePatchRect = $Menu
@onready var end_screen: NinePatchRect = $Gameover
@onready var fuel_indicator: NinePatchRect = $Tank/Level

func _ready() -> void:
	get_tree().paused = true
	end_screen.visible = false
	pause_menu.visible = true  # Ensure hidden at start
	fuel_indicator.visible = false
	update_label(0.0)

func _process(_delta: float) -> void:
	if Input.is_action_just_pressed("pause_game"):
		toggle_pause()

func toggle_pause() -> void:
	var tree = get_tree()
	tree.paused = not tree.paused
	fuel_indicator.visible = false
	pause_menu.visible = tree.paused
	
func update_label(distance: float) -> void:
	meter_label.text = "Distance: %.1f m" % distance

# This will be called when Player emits the distance_changed signal
func _on_player_distance_changed(distance: float) -> void:
	update_label(distance)

func _on_play_pressed() -> void:
	pause_menu.visible = false
	fuel_indicator.visible = true
	get_tree().paused = false

func _on_quit_pressed() -> void:
	get_tree().quit()
