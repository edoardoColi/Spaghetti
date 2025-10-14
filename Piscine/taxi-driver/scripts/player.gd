extends CharacterBody2D

const MAX_SPEED = 300.0
const ACCELERATION = 600.0
const FRICTION = 800.0

# Fuel system
var fuel: float = 100.0
const MAX_FUEL: float = 100.0
const FUEL_CONSUMPTION: float = 8.0  # fuel per second

var stunned: bool = false
var explode: bool = false

# --- Distance tracking ---
var total_distance: float = 0.0
var last_position: Vector2
signal distance_changed(distance: float)

@onready var anim_sprite: AnimatedSprite2D = $AnimatedSprite2D
@onready var collision_updown: CollisionShape2D = $CollisionUpDown
@onready var collision_leftright: CollisionShape2D = $CollisionLeftRight
@onready var fuel_level: NinePatchRect = $Camera2D/UI/Tank/Level
@onready var gameover: NinePatchRect = $Camera2D/UI/Gameover

func _ready() -> void:
	last_position = global_position

func _physics_process(delta: float) -> void:
	# --- Input only works if NOT stunned ---
	var input_vector := Vector2.ZERO
	input_vector.x = Input.get_action_strength("ui_right") - Input.get_action_strength("ui_left")
	input_vector.y = Input.get_action_strength("ui_down") - Input.get_action_strength("ui_up")
	input_vector = input_vector.normalized()

	# --- Distance tracking ---
	var moved_distance := global_position.distance_to(last_position)
	total_distance += moved_distance/30
	last_position = global_position
	distance_changed.emit(total_distance)

	# Consume fuel based on movement
	if input_vector != Vector2.ZERO and not explode:
		fuel -= FUEL_CONSUMPTION * delta
		fuel = clamp(fuel, 0, MAX_FUEL)
		update_fuel_UI()
	else:
		fuel -= MAX_FUEL/10000
		update_fuel_UI()

# If fuel empty, quit the game
	if fuel <= 0:
		gameover.visible = true
		await get_tree().create_timer(5.0).timeout
		get_tree().quit()
	
	if explode:
		# Car explode: stop and ignore input
		velocity = Vector2.ZERO
		move_and_slide()
		anim_sprite.stop() # freeze animation too
		return
	if stunned:
		# Car is stunned
		var target_speed = MAX_SPEED
		target_speed *= 0.25 # reduce to 1/4 while stunned
		velocity = velocity.move_toward(input_vector * target_speed, ACCELERATION * delta)
		move_and_slide()
		return

	# Accelerate/decelerate
	if input_vector != Vector2.ZERO:
		velocity = velocity.move_toward(input_vector * MAX_SPEED, ACCELERATION * delta)
	else:
		velocity = velocity.move_toward(Vector2.ZERO, FRICTION * delta)

	move_and_slide()

	# Animations & collision
	if input_vector != Vector2.ZERO:
		if abs(input_vector.x) > abs(input_vector.y):
			collision_leftright.disabled = false
			collision_updown.disabled = true
			if input_vector.x > 0:
				anim_sprite.play("right")
			else:
				anim_sprite.play("left")
		else:
			collision_leftright.disabled = true
			collision_updown.disabled = false
			if input_vector.y > 0:
				anim_sprite.play("down")
			else:
				anim_sprite.play("up")
	else:
		anim_sprite.stop()

# --- Stun system ---
func stun_for(duration: float) -> void:
	stunned = true
	var t = get_tree().create_timer(duration)
	t.timeout.connect(func(): stunned = false)

func explode_for(duration: float) -> void:
	explode = true
	velocity = Vector2.ZERO
	anim_sprite.stop()
	var t = get_tree().create_timer(duration)
	t.timeout.connect(func(): explode = false)

func update_fuel_UI() -> void:
	# Map fuel 0..MAX_FUEL to angle -125..-45 degrees
	var t = fuel / MAX_FUEL
	var angle_deg = lerp(-225.1, -45.1, t)  # 0 fuel -> -125, full -> -45
	fuel_level.rotation_degrees = angle_deg
