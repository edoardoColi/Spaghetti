extends CharacterBody2D

const MAX_SPEED = 300.0
const ACCELERATION = 600.0
const FRICTION = 800.0

var stunned: bool = false
var explode: bool = false

@onready var anim_sprite: AnimatedSprite2D = $AnimatedSprite2D
@onready var collision_updown: CollisionShape2D = $CollisionUpDown
@onready var collision_leftright: CollisionShape2D = $CollisionLeftRight

func _physics_process(delta: float) -> void:
	# --- Input only works if NOT stunned ---
	var input_vector := Vector2.ZERO
	input_vector.x = Input.get_action_strength("ui_right") - Input.get_action_strength("ui_left")
	input_vector.y = Input.get_action_strength("ui_down") - Input.get_action_strength("ui_up")
	input_vector = input_vector.normalized()

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
